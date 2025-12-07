#ifndef TCS34725_H
#define TCS34725_H
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Configuração I2C1 para o sensor de cor
#define TCS_I2C i2c1
#define TCS_SDA 14
#define TCS_SCL 15
#define TCS_ADDR 0x29

void tcs34725_init(void);
void tcs34725_read_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c);

#endif