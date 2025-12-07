#include "inc/TCS34725.h"

void tcs34725_init(void) {
    // Inicializa I2C1
    i2c_init(TCS_I2C, 100 * 1000);
    gpio_set_function(TCS_SDA, GPIO_FUNC_I2C);
    gpio_set_function(TCS_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(TCS_SDA);
    gpio_pull_up(TCS_SCL);

    // Habilita o sensor (Power ON + AEN)
    uint8_t cmd[2] = {0x80 | 0x00, 0x03}; 
    i2c_write_blocking(TCS_I2C, TCS_ADDR, cmd, 2, false);
    sleep_ms(50);
}

void tcs34725_read_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c) {
    // Lê a partir do registro 0x14 (CDATAL)
    uint8_t reg = 0x80 | 0x14; 
    uint8_t buffer[8];

    i2c_write_blocking(TCS_I2C, TCS_ADDR, &reg, 1, true);
    i2c_read_blocking(TCS_I2C, TCS_ADDR, buffer, 8, false);

    *c = (buffer[1] << 8) | buffer[0];
    *r = (buffer[3] << 8) | buffer[2];
    *g = (buffer[5] << 8) | buffer[4];
    *b = (buffer[7] << 8) | buffer[6];
}