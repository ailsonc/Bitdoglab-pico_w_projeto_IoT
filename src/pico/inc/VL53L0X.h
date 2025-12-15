#ifndef VL53L0X_H
#define VL53L0X_H
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Seus pinos e I2C funcional
#define VL53L0X_I2C i2c0
#define VL53L0X_SDA 0
#define VL53L0X_SCL 1
#define VL53L0X_ADDR 0x29

void vl53l0x_hardware_init(void);
int vl53l0x_sensor_init(void);
int vl53l0x_read_distance(void);

#endif