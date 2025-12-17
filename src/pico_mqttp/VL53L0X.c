#include "inc/VL53L0X.h"
#include <stdio.h>

#define REG_IDENTIFICATION_MODEL_ID 0xC0
#define REG_SYSRANGE_START 0x00
#define REG_RESULT_RANGE_STATUS 0x14
#define REG_RESULT_RANGE_MM 0x1E

void vl53l0x_hardware_init(void) {
    i2c_init(VL53L0X_I2C, 100 * 1000); // 100kHz conforme seu código
    gpio_set_function(VL53L0X_SDA, GPIO_FUNC_I2C);
    gpio_set_function(VL53L0X_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(VL53L0X_SDA);
    gpio_pull_up(VL53L0X_SCL);
}

int vl53l0x_sensor_init(void) {
    uint8_t reg = REG_IDENTIFICATION_MODEL_ID;
    uint8_t id;
    if (i2c_write_blocking(VL53L0X_I2C, VL53L0X_ADDR, &reg, 1, true) != 1) return 0;
    if (i2c_read_blocking(VL53L0X_I2C, VL53L0X_ADDR, &id, 1, false) != 1) return 0;
    return (id == 0xEE);
}

int vl53l0x_read_distance(void) {
    uint8_t cmd[2] = {REG_SYSRANGE_START, 0x01};
    i2c_write_blocking(VL53L0X_I2C, VL53L0X_ADDR, cmd, 2, false);

    // Aguarda medição
    for (int i = 0; i < 100; i++) {
        uint8_t reg = REG_RESULT_RANGE_STATUS;
        uint8_t status;
        i2c_write_blocking(VL53L0X_I2C, VL53L0X_ADDR, &reg, 1, true);
        i2c_read_blocking(VL53L0X_I2C, VL53L0X_ADDR, &status, 1, false);
        if (status & 0x01) break; 
        sleep_ms(5);
    }

    uint8_t reg_mm = REG_RESULT_RANGE_MM;
    uint8_t buffer[2];
    i2c_write_blocking(VL53L0X_I2C, VL53L0X_ADDR, &reg_mm, 1, true);
    i2c_read_blocking(VL53L0X_I2C, VL53L0X_ADDR, buffer, 2, false);

    return (buffer[0] << 8) | buffer[1];
}