#ifndef LED_RGB_H
#define LED_RGB_H
#include "pico/stdlib.h"

// Definição dos pinos (baseado no seu código)
#define LED_PIN_R 13
#define LED_PIN_G 11
#define LED_PIN_B 12

void led_rgb_init(void);
void led_set_color(uint8_t r, uint8_t g, uint8_t b);

#endif