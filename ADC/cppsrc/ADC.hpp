#ifndef ESP32_ADC_HPP
#define ESP32_ADC_HPP

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "soc/adc_channel.h"

struct ADCconfig
{
    adc_unit_t unit;
    adc1_channel_t channel;
    adc_bits_width_t width;
    adc_atten_t attenuation;
};


class ADC
{
private:
    ADCconfig *config;
    uint32_t vref;
    esp_adc_cal_characteristics_t *adc_chars;

    void ADC_config();
public:
    ADC(ADCconfig *t_config, uint32_t t_vref=1100);
    ~ADC();
    uint32_t raw(uint8_t noOfSamples=1);
};

#endif // ESP32_ADC_HPP