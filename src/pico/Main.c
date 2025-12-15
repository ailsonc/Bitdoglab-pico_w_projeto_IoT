#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h" 
#include <stdio.h>
#include <string.h>

// Seus includes
#include "inc/LED_RGB.h"
#include "inc/VL53L0X.h"
#include "inc/TCS34725.h"
#include "inc/display.h"

#define DISTANCIA_TRIGGER_MM 100 
#define AMOSTRAS_PARA_VALIDAR 5
#define BUTTON_A 5           

typedef struct {
    uint16_t distancia_mm;
    uint16_t r, g, b, c;
    bool objeto_detectado; 
} SensorData_t;

QueueHandle_t dataQueue;
SemaphoreHandle_t xI2C1Mutex; 

// --- Reset via Botão ---
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A) {
        printf("[SYSTEM] Reset forçado!\n");
        watchdog_enable(1, 1);
        while(1);
    }
}

// --- Tarefa Sensor ---
void vSensorTask(void *pvParameters) {
    SensorData_t data;
    uint8_t contador_estabilidade = 0;
    bool aguardando_retirada = false; 
    
    // ATENÇÃO: Verifique se vl53l0x usa i2c1 internamente nas suas libs
    vl53l0x_hardware_init();
    vl53l0x_sensor_init();
    
    if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
        tcs34725_init(); // Vai configurar o I2C1
        xSemaphoreGive(xI2C1Mutex);
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 50 / portTICK_PERIOD_MS; 

    for (;;) {
        uint16_t leitura_atual = vl53l0x_read_distance();
        
        if (leitura_atual > 0 && leitura_atual < DISTANCIA_TRIGGER_MM) {
            if (!aguardando_retirada) {
                if (contador_estabilidade < 255) contador_estabilidade++;
            }
        } else {
            contador_estabilidade = 0;
            if (aguardando_retirada) {
                aguardando_retirada = false;
                data.objeto_detectado = false; 
                data.distancia_mm = leitura_atual; 
                xQueueSend(dataQueue, &data, 0);
            }
        }

        if (contador_estabilidade >= AMOSTRAS_PARA_VALIDAR && !aguardando_retirada) {
            aguardando_retirada = true; 
            data.objeto_detectado = true;
            data.distancia_mm = leitura_atual;

            if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
                tcs34725_read_rgb(&data.r, &data.g, &data.b, &data.c);
                xSemaphoreGive(xI2C1Mutex);
            }
            xQueueSend(dataQueue, &data, 0);
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// --- Tarefa Processamento (Com a Normalização) ---
void vProcessingTask(void *pvParameters) {
    SensorData_t rxData;
    char lineBuffer[64];

    for (;;) {
        if (xQueueReceive(dataQueue, &rxData, portMAX_DELAY) == pdTRUE) {
            
            if (rxData.objeto_detectado) {
                
                // === O PULO DO GATO: Normalização ===
                // Transforma valores brutos (0-65535) em proporção (0-255) baseada na luz total (c)
                // Fórmula: (Cor * 255) / Clear
                
                uint8_t r_norm = 0, g_norm = 0, b_norm = 0;

                if (rxData.c > 0) { // Proteção contra divisão por zero
                    // Casting para float para manter precisão no cálculo, depois volta pra int
                    r_norm = (uint8_t)((rxData.r * 255.0f) / rxData.c);
                    g_norm = (uint8_t)((rxData.g * 255.0f) / rxData.c);
                    b_norm = (uint8_t)((rxData.b * 255.0f) / rxData.c);
                }

                // Limita em 255 caso a matemática passe um pouco (raro, mas possível)
                if (r_norm > 255) r_norm = 255;
                if (g_norm > 255) g_norm = 255;
                if (b_norm > 255) b_norm = 255;

                // --- Fim da Lógica de Normalização ---

                led_set_color(255, 255, 0); // Amarelo

                if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
                    // Mostra no display os valores já tratados
                    snprintf(lineBuffer, sizeof(lineBuffer), "R%d G%d B%d", r_norm, g_norm, b_norm);
                    display_multiline(lineBuffer, "Validando...");
                    xSemaphoreGive(xI2C1Mutex);
                }
                printf("DATA:%d,%d,%d,%d\n", rxData.r, rxData.g, rxData.b, rxData.c);

                // Aguarda resposta
                int ch_int = PICO_ERROR_TIMEOUT;
                char comando = 0;
                
                while (true) {
                    ch_int = getchar_timeout_us(100000); 
                    if (ch_int != PICO_ERROR_TIMEOUT) {
                        char c = (char)ch_int;
                        if (c == 'A' || c == 'a' || c == 'R' || c == 'r') {
                            comando = c;
                            break; 
                        }
                    }
                }

                bool aprovado = (comando == 'A' || comando == 'a');

                if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
                    if (aprovado) display_multiline("PC:", "APROVADO");
                    else display_multiline("PC:", "REPROVADO");
                    xSemaphoreGive(xI2C1Mutex);
                }

                if (aprovado) led_set_color(0, 255, 0); 
                else led_set_color(255, 0, 0);          

            } else {
                led_set_color(0, 0, 0);
                if (xSemaphoreTake(xI2C1Mutex, portMAX_DELAY) == pdTRUE) {
                    display_multiline("Pronto...", "Aguardando");
                    xSemaphoreGive(xI2C1Mutex);
                }
                printf("EVENT:CLEARED\n");
            }
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000); 

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    xI2C1Mutex = xSemaphoreCreateMutex();
    display_init(); 
    led_rgb_init();

    display_multiline("Sistema", "I2C1 Normalizado");

    dataQueue = xQueueCreate(5, sizeof(SensorData_t));

    if (dataQueue != NULL && xI2C1Mutex != NULL) {
        xTaskCreate(vSensorTask, "Sensor Task", 1024, NULL, 1, NULL);
        xTaskCreate(vProcessingTask, "Serial Task", 2048, NULL, 1, NULL);
        vTaskStartScheduler();
    }

    while (1);
    return 0;
}