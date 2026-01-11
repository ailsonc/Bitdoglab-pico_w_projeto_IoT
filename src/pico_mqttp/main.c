#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/watchdog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

#include "inc/LED_RGB.h"
#include "inc/VL53L0X.h"
#include "inc/TCS34725.h"
#include "inc/ENV.h"

/* ========================================== */

typedef enum {
    MQTT_NONE = 0,
    MQTT_APPROVED,
    MQTT_REJECTED
} MQTTResponse_t;

typedef struct {
    uint16_t r, g, b, c;
} SensorData_t;

/* ===== GLOBAIS ===== */
static mqtt_client_t *mqtt_client;
static QueueHandle_t mqttQueue;
static SemaphoreHandle_t xSemaphoreStartSensors;

static ip_addr_t broker_ip;
static bool dns_ok = false;

/* =========================================================
   FUNÇÕES AUXILIARES DE LED
   ========================================================= */
void led_red() { led_set_color(255, 0, 0); }
void led_green() { led_set_color(0, 255, 0); }
void led_yellow() { led_set_color(255, 255, 0); }
void led_off() { led_set_color(0, 0, 0); }

void blink_green_3_times() {
    for (int i = 0; i < 3; i++) {
        led_green();
        vTaskDelay(pdMS_TO_TICKS(300));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/* =========================================================
   RESET POR BOTÃO
   ========================================================= */
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A) {
        watchdog_enable(1, 1);
        while (1);
    }
}

/* =========================================================
   DNS CALLBACK
   ========================================================= */
static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *arg) {
    if (ipaddr != NULL) {
        broker_ip = *ipaddr;
        dns_ok = true;
    }
}

/* =========================================================
   MQTT CALLBACKS
   ========================================================= */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    MQTTResponse_t resp = MQTT_NONE;
    char buf[32];
    
    // Proteção de buffer e cópia segura
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, data, len);
    buf[len] = '\0';

    // Verifica palavras-chave
    if (strstr(buf, "APPROVED") || strstr(buf, "APROVADO")) {
        resp = MQTT_APPROVED;
    } else if (strstr(buf, "REJECTED") || strstr(buf, "REPROVADO")) {
        resp = MQTT_REJECTED;
    }

    if (resp != MQTT_NONE) {
        // Envia para a fila (Não bloqueante dentro da ISR/Callback)
        xQueueSend(mqttQueue, &resp, 0);
    }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    // Callback necessária para a API do lwIP
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT Conectado! (Passo 2: Verde)\n");
        led_green(); // Passo 2: Verde

        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);
        mqtt_sub_unsub(client, TOPIC_SUB, 0, NULL, NULL, 1);
        
        // Libera a MainTask
        if (xSemaphoreStartSensors != NULL) {
            xSemaphoreGive(xSemaphoreStartSensors);
        }
    } else {
        printf("Erro MQTT: %d (Passo 2: Vermelho)\n", status);
        led_red(); // Passo 2: Vermelho se falhar
    }
}

/* =========================================================
   ENVIO MQTT
   ========================================================= */
void mqtt_send_color(SensorData_t *d) {
    char payload[80];
    sprintf(payload, "{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d}", d->r, d->g, d->b, d->c);

    cyw43_arch_lwip_begin();
    mqtt_publish(mqtt_client, TOPIC_PUB, payload, strlen(payload), 0, 0, NULL, NULL);
    cyw43_arch_lwip_end();
}

/* =========================================================
   ENVIO MQTT CLEAR (REMOÇÃO)
   ========================================================= */
void mqtt_send_clear() {
    char *payload = "{\"r\":0,\"g\":0,\"b\":0,\"c\":0}";

    cyw43_arch_lwip_begin();
    mqtt_publish(mqtt_client, TOPIC_PUB, payload, strlen(payload), 0, 0, NULL, NULL);
    cyw43_arch_lwip_end();
    printf("MQTT: Status Clear enviado (Objeto Removido)\n");
}
/* =========================================================
   TASK WIFI + MQTT
   ========================================================= */
void vMQTTTask(void *pv) {
    (void)pv;
    
    // Inicialização do Hardware WiFi
    if (cyw43_arch_init()) {
        printf("Falha init cyw43\n");
        led_red();
        vTaskDelete(NULL);
    }
    cyw43_arch_enable_sta_mode();

    printf("Conectando WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_MIXED_PSK, 30000)) {
        printf("Falha WiFi (Passo 1: Vermelho)\n");
        led_red(); // Passo 1: Falha
        vTaskDelete(NULL);
    } else {
        printf("WiFi Conectado (Passo 1: Verde)\n");
        led_green(); // Passo 1: Sucesso
    }

    // Delay visual para ver o verde do WiFi antes de tentar MQTT
    vTaskDelay(pdMS_TO_TICKS(1000));
    led_off();

    // Resolução DNS
    cyw43_arch_lwip_begin();
    dns_gethostbyname(MQTT_SERVER, &broker_ip, dns_callback, NULL);
    cyw43_arch_lwip_end();

    while (!dns_ok) vTaskDelay(pdMS_TO_TICKS(100));

    // Conexão MQTT
    mqtt_client = mqtt_client_new();
    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "pico_env_v1";
    ci.keep_alive = 60;

    cyw43_arch_lwip_begin();
    // Usando BROKER_PORT corrigido
    mqtt_client_connect(mqtt_client, &broker_ip, BROKER_PORT, mqtt_connection_cb, NULL, &ci);
    cyw43_arch_lwip_end();

    // Loop infinito da tarefa
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}

/* =========================================================
   TASK PRINCIPAL (Lógica de Controle)
   ========================================================= */
void vMainTask(void *pv) {
    SensorData_t data;
    MQTTResponse_t resp;
    int verificacoes = 0; // Contador de estabilidade

    // Aguarda conexão MQTT (Sincronização via Semáforo)
    xSemaphoreTake(xSemaphoreStartSensors, portMAX_DELAY);
    
    // Passo 3: Três piscadas verdes
    printf("Sistema Pronto: Piscando Verde 3x\n");
    blink_green_3_times();

    // Inicializa sensores I2C
    vl53l0x_hardware_init();
    vl53l0x_sensor_init();
    tcs34725_init();

    while (1) {
        printf("Aguardando objeto aproximar...\n");
        led_off();
        verificacoes = 0;

        // 4. Aguardar aproximar COM FILTRO
        // O loop só sai quando detectar o objeto 5 vezes SEGUIDAS (estabilidade)
        while (verificacoes < 5) {
            uint16_t distancia = vl53l0x_read_distance();
            
            if (distancia < DISTANCIA_TRIGGER_MM && distancia > 0) {
                // Objeto detectado perto, incrementa confiança
                verificacoes++;
                printf("Detectando... %d/5 (Dist: %dmm)\n", verificacoes, distancia);
            } else {
                // Objeto longe ou leitura errada, zera a contagem
                verificacoes = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(100)); // Verifica a cada 100ms
        }

        // Se saiu do loop acima, é porque o objeto está confirmado e estável!
        printf("Objeto Confirmado! Lendo cor...\n");

        // Objeto detectado -> Amarelo
        led_yellow();
        vTaskDelay(pdMS_TO_TICKS(500)); // Estabilização para leitura de cor

        // Leitura e Envio
        tcs34725_read_rgb(&data.r, &data.g, &data.b, &data.c);
        printf("Enviando Cor: R:%d G:%d B:%d\n", data.r, data.g, data.b);
        mqtt_send_color(&data);

        // 5. Aguarda resposta por 1 minuto (60000ms)
        printf("Aguardando resposta (1 min)...\n");
        
        // Limpa a fila antes de esperar nova resposta
        xQueueReset(mqttQueue);

        if (xQueueReceive(mqttQueue, &resp, pdMS_TO_TICKS(60000))) {
            // 6. Chegou Resposta
            if (resp == MQTT_APPROVED) {
                printf("APROVADO -> Verde\n");
                led_green();
            } else {
                printf("REPROVADO -> Vermelho\n");
                led_red();
            }
        } else {
            // 5. Timeout -> Vermelho
            printf("TIMEOUT -> Vermelho\n");
            led_red();
        }

        // 6. Ficar aceso até detectar remoção
        printf("Remova o objeto para reiniciar...\n");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Garante que o usuário veja a cor antes de apagar

        // Enquanto objeto estiver perto (< Trigger), mantém o loop travado aqui
        // Também usa um pequeno filtro para não sair por erro de leitura
        while (vl53l0x_read_distance() < (DISTANCIA_TRIGGER_MM + 20)) { 
            // +20mm de histerese para evitar ficar piscando se estiver na borda
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        mqtt_send_clear();
        
        // 7. Começar tudo de novo
        printf("Objeto removido. Reiniciando ciclo.\n");
        led_off();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* =========================================================
   MAIN
   ========================================================= */
int main(void) {
    stdio_init_all();

    // Botão de Reset
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Inicializa LEDs
    led_rgb_init();

    // Criação FreeRTOS
    mqttQueue = xQueueCreate(1, sizeof(MQTTResponse_t));
    xSemaphoreStartSensors = xSemaphoreCreateBinary();

    // Cria as tasks
    xTaskCreate(vMQTTTask, "MQTT_Task", 4096, NULL, 2, NULL);
    xTaskCreate(vMainTask, "Main_Logic", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}