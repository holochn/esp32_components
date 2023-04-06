#ifndef ADCSENSOR_HPP
#define ADCSENSOR_HPP

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"

#include <iostream>

#define ADC_SENS_DEFAULT 3900
#define ADC_SENS_MIN     250
#define ADC_SENS_MAX     3150

/**
 * @brief: struct whitch holds the ADC channel number and the learn
 *         values of the sesnor
 */
typedef struct 
{
    adc1_channel_t  channel;    /*!< ADC1 channel id */
    uint32_t        dry = 1500; /*!< sensor reading for state dry */
    uint32_t        wet = 1500; /*!< sensor reading for state wet */
} adc1_sensor_t;

/**
 * @brief: Class for reading analog voltages of a ADC sensor
 * 
 */
class AdcSensor
{
    public:
    /**
     * @brief: Constructor of class AdcSensor
     * @param: 
     *          adc_unit    The ADC unit of the sensor
     *          adc_channel The ADC channel of the sensor
     *          adc_atten   The ADC attenuation
     *          adc_width   The ADC width
     *          num_samples The number of samples for multi-sampling
     *          adc_default_vref    The ADC default reference voltage. 1100 [mV] is default
     *          min_value   The start min value for learning values
     *          max_value   The start max value for learning values
     */
    AdcSensor(adc_unit_t adc_unit, adc1_channel_t adc_channel, adc_atten_t adc_atten, 
                adc_bits_width_t adc_width, uint8_t num_samples, uint32_t adc_default_vref=1100, 
                uint32_t min_value = ADC_SENS_MIN, uint32_t max_value = ADC_SENS_MAX);
    
    /**
     * @brief: Reads num_samples from ADC channel and returns the aritmetic mean
     * @param:
     * @return: The aritmetic mean of the read ADC values as uint32_t
     */
    uint32_t getVoltage();

    /**
     * @brief: Reads raw voltage from ADC channel
     * @param:
     * @return: The raw ADC voltage value as int32_t or -1 on error
     */
    int32_t getVoltageRaw();
    
    /**
     * @brief: Calculates the relative reading 
     *         (mapping from range [min_value:max_value] to [0:100])
     * @param: 
     * @return: The mapped reading. Where 0V = 100% and 3.3V = 0%
     */ 
    uint8_t getValue();

    /**
     * @brief: Getter for min_value
     * @return: The min_value as uint32_t
     */
    uint32_t getMin() const;

    /**
     * @brief: Getter for max_value
     * @return: The max_value as uint32_t
     */
    uint32_t getMax() const;

    /**
     * @brief: Activate usage of NVS to store the lerarn values to
     * @param: 
     *          nvs_ns  the NVS namespace as const char*
     * @return:
     */
    void useNvs(const char* nvs_ns);

    /**
     * @brief: Reset the learn values to default, if NVS usage is activated
     * @param: 
     * @return:
     */
    void resetNvs();

    private:
    void sensor_learn_values(uint32_t reading);
    void calibrate();
    bool storeValsToNVS;
    std::string nvs_ns;
    std::string nvs_ns_min;
    std::string nvs_ns_max;
    adc_unit_t unit;
    adc1_channel_t channel;
    adc_atten_t atten;
    adc_bits_width_t width;
    uint32_t default_vref;
    uint32_t sensor_min;
    uint32_t sensor_max;
    uint8_t samples;
    esp_adc_cal_characteristics_t *adc_chars;
    uint8_t cal_count;
};

#endif // ADCSENSOR_HPP