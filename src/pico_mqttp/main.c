#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/apps/mqtt.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"

// ---------------- CONFIG ----------------
#define WIFI_SSID   "Lemos"
#define WIFI_PASS   "a1b2c3d4@2!3500"

#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT   1883

#define TOPIC_PUB   "bitdoglab/send"
#define TOPIC_SUB   "bitdoglab/recv"

// ----------------------------------------
static mqtt_client_t *mqtt_client;
static ip_addr_t broker_ip;
static bool dns_resolved = false;

// -------- DNS CALLBACK --------
static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr) {
        broker_ip = *ipaddr;
        dns_resolved = true;
        printf("DNS OK: %s -> %s\n", name, ipaddr_ntoa(ipaddr));
    } else {
        printf("DNS falhou para %s\n", name);
    }
}

// -------- MQTT CALLBACKS --------
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Mensagem recebida: %.*s\n", len, data);
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Tópico: %s\n", topic);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
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

// -------- TASK PRINCIPAL --------
void main_task(void *params) {

    printf("Inicializando Wi-Fi...\n");

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar Wi-Fi\n");
        vTaskDelete(NULL);
    }

    cyw43_arch_enable_sta_mode();

    cyw43_wifi_pm(
        &cyw43_state,
        cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1)
    );

    printf("Conectando ao Wi-Fi: %s\n", WIFI_SSID);

    int err = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID,
        WIFI_PASS,
        CYW43_AUTH_WPA2_MIXED_PSK,
        60000
    );

    if (err) {
        printf("Erro Wi-Fi: %d\n", err);
        vTaskDelete(NULL);
    }

    printf("Wi-Fi conectado. Aguardando DHCP...\n");

    struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];

    for (int i = 0; i < 20; i++) {
        if (!ip4_addr_isany_val(*netif_ip4_addr(netif))) {
            printf("IP OK!\n");
            printf("IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
            printf("GW: %s\n", ip4addr_ntoa(netif_ip4_gw(netif)));
            printf("MASK: %s\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (ip4_addr_isany_val(*netif_ip4_addr(netif))) {
        printf("DHCP falhou\n");
        vTaskDelete(NULL);
    }

    // -------- DNS RESOLVE --------
    printf("Resolvendo DNS: %s\n", MQTT_SERVER);

    cyw43_arch_lwip_begin();
    err_t dns_err = dns_gethostbyname(
        MQTT_SERVER,
        &broker_ip,
        dns_callback,
        NULL
    );
    cyw43_arch_lwip_end();

    if (dns_err == ERR_OK) {
        dns_resolved = true;
        printf("DNS imediato: %s\n", ipaddr_ntoa(&broker_ip));
    } else if (dns_err != ERR_INPROGRESS) {
        printf("Erro DNS: %d\n", dns_err);
        vTaskDelete(NULL);
    }

    while (!dns_resolved) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // -------- MQTT CONNECT --------
    mqtt_client = mqtt_client_new();

    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "pico_w_dns";
    ci.keep_alive = 60;

    printf("Conectando MQTT em %s...\n", ipaddr_ntoa(&broker_ip));

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

    int count = 0;
    char msg[64];

    while (true) {
        sprintf(msg, "Hello MQTT DNS %d", count++);

        cyw43_arch_lwip_begin();
        mqtt_publish(
            mqtt_client,
            TOPIC_PUB,
            msg,
            strlen(msg),
            0,
            0,
            NULL,
            NULL
        );
        cyw43_arch_lwip_end();

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

int main(void) {
    stdio_init_all();

    xTaskCreate(
        main_task,
        "MainTask",
        6144,
        NULL,
        2,
        NULL
    );

    vTaskStartScheduler();
    while (1);
}
