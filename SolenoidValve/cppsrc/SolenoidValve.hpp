#ifndef SOLENOID_VALVE_HPP
#define SOLENOID_VALVE_HPP

#include <iostream>
#include <memory>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"

#define SolenoidValve_SwitchTime_MS 2500

using std::string;
using std::shared_ptr;

typedef enum {
    SOLENOID_VALVE_STATE_OFF = 0,
    SOLENOID_VALVE_STATE_ON
} solenoid_valve_state_t;

class SolenoidValve
{
public:
    /**
     * @brief: Constructor of class SolenoidValve
     * @param:
     *          name std::string name of the valve
     *          t_enable enable gpio number as uint8_t
     *          t_pin_in1 rotation direction 1 gpio number as uint8_t
     *          t_pin_in2 rotation direction 2 gpio number as uint8_t
     *          nvs_ns nvs name space for storing the valve state
     */
    SolenoidValve(string name, uint8_t pin_enable_, 
                    uint8_t t_pin_in1, uint8_t t_pin_in2, 
                    string nvs_ns);

    /**
     * @brief: Toggles the state of the valve
     * @return: ESP_OK on success
     */
    esp_err_t toggleValveState();

    /**
     * @brief: Toggles the state of the valve
     * @param: The new state of the valve as uint8_t
     * @return: ESP_OK on success
     */
    esp_err_t setValveState(uint8_t new_state);

    /**
     * @brief: Sets the enable pin of the valve to FALSE
     * @param: 
     * @return: 
     */
    void release();

    private:
    const char* TAG = "SolenoidValve";   
    std::string name_;
    std::string nvs_ns_;
    gpio_num_t m_pin_enable;
    gpio_num_t m_pin_in1;
    gpio_num_t m_pin_in2;
    uint8_t valve_state_;
    shared_ptr<nvs::NVSHandle> handle_;
};

#endif // SOLENOID_VALVE_HPP