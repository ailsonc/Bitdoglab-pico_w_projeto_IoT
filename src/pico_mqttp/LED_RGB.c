#include "inc/LED_RGB.h"
#include "hardware/pwm.h"

void set_pwm_level(uint pin, uint16_t level) {
    // Gama correction simples (level * level) conforme seu código original
    pwm_set_gpio_level(pin, level * level);
}

void led_rgb_init(void) {
    uint pins[] = {LED_PIN_R, LED_PIN_G, LED_PIN_B};
    
    for (int i = 0; i < 3; i++) {
        gpio_set_function(pins[i], GPIO_FUNC_PWM);
        uint slice = pwm_gpio_to_slice_num(pins[i]);
        pwm_set_wrap(slice, 65535); // Resolução máxima 16-bit
        pwm_set_enabled(slice, true);
    }
}

void led_set_color(uint8_t r, uint8_t g, uint8_t b) {
    set_pwm_level(LED_PIN_R, r);
    set_pwm_level(LED_PIN_G, g);
    set_pwm_level(LED_PIN_B, b);
}