#ifndef ESP32_DHT22_HPP
#define ESP32_DHT22_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include <iostream>

const uint8_t DHT22_TIMEOUT                   = 0xFF;   
const uint8_t DHT22_MCU_PULLUP_START_TIME_US  = 40;
const uint8_t DHT22_SENS_SIGNAL_START_TIME_US = 85;
const uint8_t DHT22_SENS_SIGNAL_PREBIT_TIME_US= 55;
const uint8_t DHT22_SENS_SIGNAL_ONE_TIME_US   = 40;
const uint8_t DHT22_SENS_SIGNAL_ZERO_MIN_TIME_US  = 26;
const uint8_t DHT22_SENS_SIGNAL_ZERO_MAX_TIME_US  = 28;

class DHT22
{
private:
    gpio_num_t m_data_pin;
    uint8_t data[5]{0};
    int8_t expectSignalCycles(bool expected_value, uint8_t max, uint8_t intervall=1);
public:
    DHT22(uint8_t t_data_pin);
    ~DHT22();
    void read();
    float getHumidity();
    float getTemperature();
    
};

#endif // ESP32_DHT22_HPP