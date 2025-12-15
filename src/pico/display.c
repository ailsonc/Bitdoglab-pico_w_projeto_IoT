#include "inc/display.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include <string.h>
#include <stdio.h>

static struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1
};

// Inicialização do I2C e do display SSD1306
void display_init(void) {
    i2c_init(SSD1306_I2C, SSD1306_I2C_CLOCK * 1000);
    gpio_set_function(SSD1306_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(SSD1306_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SSD1306_I2C_SDA);
    gpio_pull_up(SSD1306_I2C_SCL);

    ssd1306_init();

    ssd1306_send_command(ssd1306_set_display | 0x01);

    calculate_render_area_buffer_length(&frame_area);
}

void display_status_msg(const char status_msg[]) {
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    char status_text[128];
    snprintf(status_text, sizeof(status_text), "%s", status_msg);
    ssd1306_draw_string(ssd, 5, 0, status_text);
    render_on_display(ssd, &frame_area);
}

void display_multiline(const char* line1, const char* line2) {
    // Buffer para a tela
    uint8_t ssd[ssd1306_buffer_length];
    
    // 1. Limpa o buffer completamente com zeros (tela preta)
    memset(ssd, 0, ssd1306_buffer_length);

    // 2. Desenha a primeira linha (se ela não for nula)
    // Posição: X=5, Y=0 (topo)
    if (line1 != NULL) {
        ssd1306_draw_string(ssd, 5, 0, (char*)line1);
    }

    // 3. Desenha a segunda linha (se ela não for nula)
    // Posição: X=5, Y=12 (um pouco abaixo da primeira)
    // A altura da fonte padrão é 8 pixels, então 12 dá um bom espaçamento.
    if (line2 != NULL) {
        ssd1306_draw_string(ssd, 5, 12, (char*)line2);
    }
    
    render_on_display(ssd, &frame_area);
}
