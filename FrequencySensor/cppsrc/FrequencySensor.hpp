#ifndef FREQUENCYSENSOR_HPP
#define FREQUENCYSENSOR_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"

typedef struct 
{
    void* ptrFS;
    portMUX_TYPE* mux;

} frequencysensor_evt_handl_arg_t;

void pcnt_get_counter(void* p);

/**
 * @brief: Class to meassure a frequency
 */
class FrequencySensor
{
    public:
    /**
     * @brief: Constructor of class FrequencySensor
     * @param: 
     *         config Pointer to a PCNT configuration pcnt_config_t
     *         min_val The start min value for learning values. Default is 0xFFFFFFFF
     *         max_val The start max value for learning values. Default is 0
     *         sample_time  The sample time in ms. Default is 100 ms
     */
    FrequencySensor(pcnt_config_t *config, 
                    uint32_t min_val = 0xFFFFFFFF, 
                    uint32_t max_val = 0,
                    uint64_t sample_time = 100);

    /**
     * @brief: Increment the overflow counter in the event handler
     */
    void incOverflowCounter();

    /**
     * @brief: Getter for the overflow counter
     * @return: the overflow counter value as uint16_t
     */
    uint16_t getOverflowCounter() const;

    /**
     * @brief: Setter for the overflow counter. Mainly used for resetting to zero
     * @param: The value to which overflow counter should be set as uint16_t
     */
    void setOverflowCounter(uint16_t val);

    /**
     * @brief: Calculates the frequency: PCNT counter * (overflow counter * hLim)
     */
    void calcFrequency(const uint16_t pcnt_counter);
    
    /**
     * @brief: Getter for frequency
     * @return: The frequency value as double
     */
    double getFrequency() const;

    /**
     * @brief: Getter for hLim
     * @return: The hLim value as uint16_t
     */
    int16_t getHlim() const;

    /**
     * @brief: Getter for pulse count unit
     * @return: The pulse count unitas pcnt_unit_t
     */
    pcnt_unit_t getPcntUnit() const;

    /**
     * @brief: Static class method for reading the pulse counter value
     * @param: Pointer where to store the pulse counter value
     */
    static void pcnt_get_counter(void* p);

    protected:
    frequencysensor_evt_handl_arg_t evnt_handle_arg;
    esp_timer_create_args_t timer_args;
    esp_timer_handle_t timer_handle;
    uint16_t overflow_count;
    uint32_t sensor_min;
    uint32_t sensor_max;
    uint64_t sensor_sample_time;
    double frequency;
    pcnt_config_t *pcnt_config;
    portMUX_TYPE timer_mux;
};

#endif // FREQUENCYSENSOR_HPP