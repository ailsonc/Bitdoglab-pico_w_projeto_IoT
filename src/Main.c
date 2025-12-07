#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h" // Necessário para o Mutex
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#include "inc/LED_RGB.h"
#include "inc/VL53L0X.h"
#include "inc/TCS34725.h"
#include "display.h"

// --- Configurações ---
#define DISTANCIA_TRIGGER_MM 100 // Distância para detectar (ajustei para 10cm p/ teste)
#define AMOSTRAS_PARA_VALIDAR 5

// Estrutura de dados
typedef struct {
    uint16_t distancia_mm;
    uint16_t r, g, b, c;
    bool objeto_detectado; // True = Objeto novo, False = Objeto removido
} SensorData_t;

QueueHandle_t dataQueue;
SemaphoreHandle_t xI2C1Mutex; // O "Guarda de Trânsito" do I2C1

// --- Validação de Cor ---
bool validar_cor(uint16_t r, uint16_t g, uint16_t b) {
    // Lógica simples: Verde deve ser maior que Vermelho e Azul
    if (g > r && g > b) return true; 
    return false;
}

// --- Tarefa 1: Sensor (Leitura e Lógica de Estado) ---
void vSensorTask(void *pvParameters) {
    SensorData_t data;
    uint8_t contador_estabilidade = 0;
    bool aguardando_retirada = false; // Flag para impedir leituras repetidas do mesmo objeto
    
    // Inicializa hardware
    vl53l0x_hardware_init();
    vl53l0x_sensor_init();
    
    // Protege a inicialização do I2C1 (Cor + Display)
    if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
        tcs34725_init();
        xSemaphoreGive(xI2C1Mutex);
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 50 / portTICK_PERIOD_MS; 

    for (;;) {
        // 1. Lê Distância (I2C0 - Seguro, sem conflito)
        uint16_t leitura_atual = vl53l0x_read_distance();
        
        // 2. Lógica de Detecção
        if (leitura_atual > 0 && leitura_atual < DISTANCIA_TRIGGER_MM) {
            // Objeto está perto
            if (!aguardando_retirada) {
                // Ainda não processamos este objeto, conta estabilidade
                if (contador_estabilidade < 255) contador_estabilidade++;
            }
        } else {
            // Objeto saiu (longe)
            contador_estabilidade = 0;
            
            // Se o objeto acabou de sair, avisa o sistema para resetar
            if (aguardando_retirada) {
                aguardando_retirada = false;
                
                // Envia aviso que limpou a área
                data.objeto_detectado = false; 
                data.distancia_mm = leitura_atual; // Deve ser alto
                xQueueSend(dataQueue, &data, 0);
            }
        }

        // 3. Disparo da Leitura (Apenas se atingiu 5 amostras e não foi processado ainda)
        if (contador_estabilidade >= AMOSTRAS_PARA_VALIDAR && !aguardando_retirada) {
            
            // Bloqueia a flag para não ler de novo até tirar o objeto
            aguardando_retirada = true; 
            
            data.objeto_detectado = true;
            data.distancia_mm = leitura_atual;

            // --- REGIÃO CRÍTICA (Uso do I2C1) ---
            // Tenta pegar a chave do I2C1
            if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
                // Com a chave na mão, lemos a cor
                tcs34725_read_rgb(&data.r, &data.g, &data.b, &data.c);
                // Devolve a chave
                xSemaphoreGive(xI2C1Mutex);
            }
            // ------------------------------------

            // Envia para processamento
            xQueueSend(dataQueue, &data, 0);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// --- Tarefa 2: Processamento, Display e Feedback ---
void vProcessingTask(void *pvParameters) {
    SensorData_t rxData;
    char line1[32];
    char line2[32];

    for (;;) {
        if (xQueueReceive(dataQueue, &rxData, portMAX_DELAY) == pdTRUE) {
            
            if (rxData.objeto_detectado) {
                // === NOVO OBJETO DETECTADO ===
                
                printf("[PROCESSO] Celular detectado a %d mm.\n", rxData.distancia_mm);

                // 1. Validação (Cor)
                bool aprovado = validar_cor(rxData.r, rxData.g, rxData.b);
                
                // 2. Atualiza Display (Protegido por Mutex)
                if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
                    snprintf(line1, sizeof(line1), "Dist: %d mm", rxData.distancia_mm);
                    
                    if (aprovado) snprintf(line2, sizeof(line2), "APROVADO (VERDE)");
                    else snprintf(line2, sizeof(line2), "REPROVADO");

                    display_multiline(line1, line2);
                    xSemaphoreGive(xI2C1Mutex);
                }

                // 3. Feedback LED
                if (aprovado) led_set_color(0, 255, 0); // Verde
                else led_set_color(255, 0, 0);          // Vermelho

                // 4. Envia MQTT (Simulação)
                printf("[MQTT] Enviando JSON: {status: %s, r:%d, g:%d, b:%d}\n", 
                        aprovado ? "OK" : "FAIL", rxData.r, rxData.g, rxData.b);

            } else {
                // === OBJETO REMOVIDO ===
                printf("[SISTEMA] Aguardando proximo item...\n");
                
                // Reseta LED
                led_set_color(0, 0, 20); // Azul fraco (Standby)

                // Atualiza Display (Protegido por Mutex)
                if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
                    display_multiline("Pronto...", "Insira Celular");
                    xSemaphoreGive(xI2C1Mutex);
                }
            }
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000); 

    // Cria o Mutex para proteger o I2C1
    xI2C1Mutex = xSemaphoreCreateMutex();

    // Inicializa I2C1 e Display (Protegido, embora aqui ainda não tenha tasks rodando)
    display_init(); 
    display_multiline("Iniciando", "Sistema...");

    led_rgb_init();

    dataQueue = xQueueCreate(5, sizeof(SensorData_t));

    if (dataQueue != NULL && xI2C1Mutex != NULL) {
        xTaskCreate(vSensorTask, "Sensor Task", 1024, NULL, 1, NULL);
        xTaskCreate(vProcessingTask, "Logic Task", 1024, NULL, 1, NULL);
        vTaskStartScheduler();
    }

    while (1);
    return 0;
}