#ifndef PUMP_HPP
#define PUMP_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

class Pump
{
    public:
    Pump(uint8_t gpio_num);
    void start(const uint8_t pwm_duty_cycle=100);
    void set_duty(const uint8_t pwm_duty_cycle=100);
    void stop();

    private:
    const char* TAG = "Pump";
    gpio_num_t pin;
    uint8_t current_duty_cycle;
    ledc_timer_config_t ledc_timer;
    ledc_channel_config_t ledc_channel;
};

#endif // PUMP_HPP