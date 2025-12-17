#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/watchdog.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

#include "inc/LED_RGB.h"
#include "inc/VL53L0X.h"
#include "inc/TCS34725.h"
#include "inc/ENV.h"

#define DISTANCIA_TRIGGER_MM 100
#define AMOSTRAS_PARA_VALIDAR 5
#define BUTTON_A 5
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

static ip_addr_t broker_ip;
static bool dns_ok = false;

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
static void dns_callback(const char *name,
                         const ip_addr_t *ipaddr,
                         void *arg) {
    if (ipaddr != NULL) {
        broker_ip = *ipaddr;
        dns_ok = true;
    }
}

/* =========================================================
   MQTT RX CALLBACK
   ========================================================= */
static void mqtt_incoming_data_cb(void *arg,
                                  const u8_t *data,
                                  u16_t len,
                                  u8_t flags) {

    MQTTResponse_t resp = MQTT_NONE;

    if (strncmp((char *)data, "APPROVED", len) == 0)
        resp = MQTT_APPROVED;
    else if (strncmp((char *)data, "REJECTED", len) == 0)
        resp = MQTT_REJECTED;

    if (resp != MQTT_NONE) {
        xQueueSend(mqttQueue, &resp, 0);
    }
}

static void mqtt_incoming_publish_cb(void *arg,
                                     const char *topic,
                                     u32_t tot_len) {
    (void)arg; (void)topic; (void)tot_len;
}

static void mqtt_connection_cb(mqtt_client_t *client,
                               void *arg,
                               mqtt_connection_status_t status) {

    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT conectado!\n");

        mqtt_set_inpub_callback(
            client,
            mqtt_incoming_publish_cb,
            mqtt_incoming_data_cb,
            NULL
        );

        mqtt_sub_unsub(client, TOPIC_SUB, 0, NULL, NULL, 1);
    } else {
        printf("Erro MQTT: %d\n", status);
    }
}

/* =========================================================
   ENVIO MQTT
   ========================================================= */
void mqtt_send_color(SensorData_t *d) {

    char payload[80];

    sprintf(payload,
            "{\"r\":%d,\"g\":%d,\"b\":%d,\"c\":%d}",
            d->r, d->g, d->b, d->c);

    cyw43_arch_lwip_begin();
    mqtt_publish(
        mqtt_client,
        TOPIC_PUB,
        payload,
        strlen(payload),
        0,
        0,
        NULL,
        NULL
    );
    cyw43_arch_lwip_end();
}

/* =========================================================
   TASK WIFI + MQTT
   ========================================================= */
void vMQTTTask(void *pv) {

    (void)pv;

    printf("Inicializando Wi-Fi...\n");

    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();

    cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID,
        WIFI_PASS,
        CYW43_AUTH_WPA2_MIXED_PSK,
        60000
    );

    /* ===== DHCP ===== */
    struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];

    printf("Aguardando DHCP...\n");

    for (int i = 0; i < 30; i++) {
        if (!ip4_addr_isany_val(*netif_ip4_addr(netif))) {
            printf("IP obtido!\n");
            printf("IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
            printf("GW: %s\n", ip4addr_ntoa(netif_ip4_gw(netif)));
            printf("MASK: %s\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (ip4_addr_isany_val(*netif_ip4_addr(netif))) {
        led_set_color(255, 0, 0);  // Vermelho
        printf("DHCP falhou!\n");
        vTaskDelete(NULL);
    }

    /* ===== DNS ===== */
    printf("Resolvendo DNS: %s\n", MQTT_SERVER);

    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname(
        MQTT_SERVER,
        &broker_ip,
        dns_callback,
        NULL
    );
    cyw43_arch_lwip_end();

    if (err == ERR_OK) {
        dns_ok = true;
        printf("DNS imediato OK\n");
    }

    while (!dns_ok) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    printf("DNS resolvido: %s\n", ipaddr_ntoa(&broker_ip));

    led_set_color(0, 255, 0);   // Verde

    /* ===== MQTT CONNECT ===== */
    mqtt_client = mqtt_client_new();

    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "pico_color";
    ci.keep_alive = 60;

    cyw43_arch_lwip_begin();
    mqtt_client_connect(
        mqtt_client,
        &broker_ip,
        MQTT_PORT,
        mqtt_connection_cb,
        NULL,
        &ci
    );
    cyw43_arch_lwip_end();

    led_set_color(0, 0, 0);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* =========================================================
   TASK PRINCIPAL – SENSOR + PROCESSO
   ========================================================= */
void vMainTask(void *pv) {

    (void)pv;

    SensorData_t data;
    MQTTResponse_t resp;

    uint8_t estabilidade = 0;
    bool objeto_presente = false;
    bool resultado_exibido = false;

    vl53l0x_hardware_init();
    vl53l0x_sensor_init();
    tcs34725_init();

    while (1) { 

        int distancia = vl53l0x_read_distance();

        /* ================================
           OBJETO PRESENTE
           ================================ */
        if (distancia > 0 && distancia < DISTANCIA_TRIGGER_MM) {

            if (!objeto_presente && !resultado_exibido) {

                if (++estabilidade >= AMOSTRAS_PARA_VALIDAR) {

                    objeto_presente = true;
                    estabilidade = 0;

                    /* LED AMARELO */
                    led_set_color(255, 255, 0);

                    /* Leitura da cor */
                    tcs34725_read_rgb(
                        &data.r,
                        &data.g,
                        &data.b,
                        &data.c
                    );

                    /* Envia MQTT */
                    mqtt_send_color(&data);

                    /* Aguarda resposta */
                    if (xQueueReceive(
                            mqttQueue,
                            &resp,
                            pdMS_TO_TICKS(60000))) {

                        if (resp == MQTT_APPROVED) {
                            led_set_color(0, 255, 0);   // Verde
                        } else {
                            led_set_color(255, 0, 0);  // Vermelho
                        }

                    } else {
                        led_set_color(255, 0, 0);      // Timeout
                    }

                    resultado_exibido = true;
                }
            }
        }
        /* ================================
           OBJETO REMOVIDO → RESET DO CICLO
           ================================ */
        else {

            estabilidade = 0;
            objeto_presente = false;

            if (resultado_exibido) {
                /* Só aqui o sistema reseta */
                resultado_exibido = false;
                led_set_color(0, 0, 0); // LED apagado
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }

}

/* =========================================================
   MAIN
   ========================================================= */
int main(void) {

    stdio_init_all();

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(
        BUTTON_A,
        GPIO_IRQ_EDGE_FALL,
        true,
        &gpio_callback
    );

    led_rgb_init();

    mqttQueue = xQueueCreate(3, sizeof(MQTTResponse_t));

    xTaskCreate(vMQTTTask, "MQTT", 4096, NULL, 2, NULL);
    xTaskCreate(vMainTask, "MAIN", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}
