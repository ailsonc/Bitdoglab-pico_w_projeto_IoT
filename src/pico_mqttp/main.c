#include <stdio.h>
#include <string.h>
#include <stdbool.h> // Necessário para true/false em C
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/apps/mqtt.h"

// --- Configurações ---
#define WIFI_SSID "Lemos"
#define WIFI_PASS "a1b2c3d4@2!3500"
#define MQTT_SERVER "broker.hivemq.com" // IP do Broker
#define BROKER_PORT 1883

// Tópicos
#define TOPIC_PUB "bitdoglab/send"
#define TOPIC_SUB "bitdoglab/recv"

// Variáveis Globais
static mqtt_client_t* mqtt_client;

// --- Callbacks do MQTT ---

// Callback para dados recebidos (payload)
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Mensagem Recebida: '%.*s'\n", len, data);
    
    // Exemplo: Controle de LED (verifique o pino correto da sua BitDogLab)
    // if (strncmp((char*)data, "ON", len) == 0) {
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    // }
}

// Callback para quando chega um novo publish (tópico)
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Tópico recebido: %s | Tamanho: %d\n", topic, tot_len);
}

// Callback de estado da conexão
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT Conectado! Inscrevendo-se em %s...\n", TOPIC_SUB);
        
        // Configura os callbacks de recebimento antes de assinar
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);
        
        // Assina o tópico
        err_t err = mqtt_sub_unsub(client, TOPIC_SUB, 0, NULL, NULL, 1);
        if (err != ERR_OK) {
            printf("Erro ao assinar tópico: %d\n", err);
        }
    } else {
        printf("Falha na conexão MQTT. Status: %d\n", status);
    }
}

// --- Função de Publicação ---

void publish_message(const char* msg) {
    if (mqtt_client_is_connected(mqtt_client)) {
        // Bloqueio necessário para operações lwIP em FreeRTOS
        cyw43_arch_lwip_begin();
        
        u16_t err = mqtt_publish(mqtt_client, TOPIC_PUB, msg, strlen(msg), 0, 0, NULL, NULL);
        
        cyw43_arch_lwip_end();
        
        if (err != ERR_OK) {
            printf("Erro ao publicar: %d\n", err);
        } else {
            printf("Publicado: %s\n", msg);
        }
    } else {
        printf("MQTT desconectado. Tentando reconectar...\n");
        // Lógica de reconexão poderia ser adicionada aqui
    }
}

// --- Tarefa Principal do FreeRTOS ---
void main_task(void *params) {
    printf("Inicializando Wi-Fi...\n");

    if (cyw43_arch_init()) {
        printf("Falha ao iniciar hardware Wi-Fi.\n");
        vTaskDelete(NULL);
    }

    cyw43_arch_enable_sta_mode();

    // --- CORREÇÃO DO ERRO -4 (SEM A LINHA DO PAÍS) ---
    printf("Desativando modo de economia de energia...\n");
    
    // Isso impede que o chip Wi-Fi "durma" e cause o erro -4 ou -2
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));

    // -------------------------------------------------
    
    printf("Wi-Fi Inicializado. Preparando scan...\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("Conectando ao Wi-Fi: %s...\n", WIFI_SSID);

    struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];

    // Defina o IP, Máscara e Gateway (ajuste para sua rede)
    ip4_addr_t ip, mask, gw;
    IP4_ADDR(&ip, 192, 168, 1, 200);    // Escolha um IP livre (ex: final 55)
    IP4_ADDR(&mask, 255, 255, 255, 0); // Máscara padrão
    IP4_ADDR(&gw, 192, 168, 1, 1);     // IP do seu roteador

    netif_set_addr(n, &ip, &mask, &gw);
    netif_set_up(n);

    // 2. Conecta ao Wi-Fi (timeout de 30s)
    int erro = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000);

    if (erro) {
        printf("Falha ao conectar. Codigo de Erro: %d\n", erro);
        vTaskDelete(NULL);
    }
    printf("Wi-Fi Conectado!\n");
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // Acende LED onboard

    // 3. Conecta ao Broker MQTT
    ip_addr_t broker_ip;
    if (!ipaddr_aton(MQTT_SERVER, &broker_ip)) {
        printf("Erro: IP do Broker inválido.\n");
    }

    mqtt_client = mqtt_client_new();
    
    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci)); // Zera a estrutura
    ci.client_id = "pico_w_c_client";
    ci.keep_alive = 60;
    
    printf("Conectando ao Broker MQTT...\n");
    
    // Bloqueia core para conexão inicial
    cyw43_arch_lwip_begin();
    mqtt_client_connect(mqtt_client, &broker_ip, MQTT_PORT, mqtt_connection_cb, NULL, &ci);
    cyw43_arch_lwip_end();

    int count = 0;
    char buffer[64];

    // 4. Loop Infinito
    while (true) {
        sprintf(buffer, "Contador C FreeRTOS: %d", count++);
        publish_message(buffer);
        
        // Delay de 5 segundos (convertido para ticks)
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

int main(void) {
    stdio_init_all();
    
    // Cria a tarefa (Stack Size 4096 bytes, Prioridade 1)
    xTaskCreate(main_task, "MainTask", 4096, NULL, 1, NULL);

    // Inicia o escalonador
    vTaskStartScheduler();

    while (true) {}; // Loop de segurança
}