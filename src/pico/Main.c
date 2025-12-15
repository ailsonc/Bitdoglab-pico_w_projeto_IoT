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

#define DISTANCIA_TRIGGER_MM 100 
#define AMOSTRAS_PARA_VALIDAR 5
#define BUTTON_A 5           

typedef struct {
    uint16_t distancia_mm;
    uint16_t r, g, b, c;
    bool objeto_detectado; 
} SensorData_t;

QueueHandle_t dataQueue;
// REMOVIDO: SemaphoreHandle_t xI2C1Mutex;  <-- Não precisamos mais disso

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
    
    // Inicializa hardware
    vl53l0x_hardware_init();
    vl53l0x_sensor_init();
    
    // Inicializa Sensor de Cor (Sem Mutex)
    tcs34725_init(); 

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

            // Leitura direta (Sem Mutex, pois o display está desligado)
            tcs34725_read_rgb(&data.r, &data.g, &data.b, &data.c);
            
            xQueueSend(dataQueue, &data, 0);
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// --- Tarefa Processamento ---
void vProcessingTask(void *pvParameters) {
    SensorData_t rxData;

    for (;;) {
        if (xQueueReceive(dataQueue, &rxData, portMAX_DELAY) == pdTRUE) {
            
            if (rxData.objeto_detectado) {
                
                // Liga Led Amarelo
                led_set_color(255, 255, 0); 

                // Envia dados BRUTOS para o PC fazer a matemática
                // Formato: DATA:R,G,B,C
                printf("DATA:%d,%d,%d,%d\n", rxData.r, rxData.g, rxData.b, rxData.c);

                // Aguarda resposta do C#
                int ch_int = PICO_ERROR_TIMEOUT;
                char comando = 0;
                
                // Timeout curto de leitura
                int tries = 0;
                while (tries < 50) { // Tenta por aprox 5 segundos (50 * 100ms)
                    ch_int = getchar_timeout_us(100000); 
                    if (ch_int != PICO_ERROR_TIMEOUT) {
                        char c = (char)ch_int;
                        if (c == 'A' || c == 'a' || c == 'R' || c == 'r') {
                            comando = c;
                            break; 
                        }
                    }
                    tries++;
                }

                bool aprovado = (comando == 'A' || comando == 'a');

                if (aprovado) led_set_color(0, 255, 0); // Verde
                else led_set_color(255, 0, 0);          // Vermelho

            } else {
                led_set_color(0, 0, 0);
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

    led_rgb_init();

    dataQueue = xQueueCreate(5, sizeof(SensorData_t));

    // Verificação simplificada (Sem xI2C1Mutex)
    if (dataQueue != NULL) {
        xTaskCreate(vSensorTask, "Sensor Task", 1024, NULL, 1, NULL);
        xTaskCreate(vProcessingTask, "Serial Task", 2048, NULL, 1, NULL);
        vTaskStartScheduler();
    }

    while (1);
    return 0;
}