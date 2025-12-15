#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define SSD1306_I2C i2c1
#define SSD1306_I2C_SDA 14
#define SSD1306_I2C_SCL 15
#define SSD1306_I2C_CLOCK 400

void display_init(void);
void display_status_msg(const char status_msg[]);
void display_multiline(const char* line1, const char* line2);

#endif