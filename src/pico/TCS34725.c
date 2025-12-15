#include "inc/TCS34725.h"
#include "hardware/i2c.h"

void tcs34725_init(void) {
    // Inicializa I2C usando a definição do header (TCS_I2C)
    i2c_init(TCS_I2C, 100 * 1000);
    
    gpio_set_function(TCS_SDA, GPIO_FUNC_I2C);
    gpio_set_function(TCS_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(TCS_SDA);
    gpio_pull_up(TCS_SCL);

    // === Configurações Anti-Saturação ===
    // Integração Rápida (2.4ms = 0xFF) - Modo "Óculos Escuros"
    uint8_t cmd_time[2] = {0x80 | 0x01, 0xFF}; 
    i2c_write_blocking(TCS_I2C, TCS_ADDR, cmd_time, 2, false);

    // Ganho Mínimo (1x)
    uint8_t cmd_gain[2] = {0x80 | 0x0F, 0x00}; 
    i2c_write_blocking(TCS_I2C, TCS_ADDR, cmd_gain, 2, false);

    // Liga o Sensor
    uint8_t cmd_enable[2] = {0x80 | 0x00, 0x03}; 
    i2c_write_blocking(TCS_I2C, TCS_ADDR, cmd_enable, 2, false);
    
    sleep_ms(50);
}

void tcs34725_read_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c) {
    uint8_t reg = 0x80 | 0x14; 
    uint8_t buffer[8];

    // Usa TCS_I2C definido no header
    i2c_write_blocking(TCS_I2C, TCS_ADDR, &reg, 1, true);
    i2c_read_blocking(TCS_I2C, TCS_ADDR, buffer, 8, false);

    *c = (buffer[1] << 8) | buffer[0];
    *r = (buffer[3] << 8) | buffer[2];
    *g = (buffer[5] << 8) | buffer[4];
    *b = (buffer[7] << 8) | buffer[6];
}