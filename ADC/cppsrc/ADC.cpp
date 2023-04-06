#include "ADC.hpp"

ADC::ADC(ADCconfig *t_config, uint32_t t_vref) :
            config{t_config}, vref{t_vref}
{
    ADC_config();
}

void ADC::ADC_config()
{
    //Configure ADC
    if (config->unit == ADC_UNIT_1) {
        adc1_config_width(config->width);
        adc1_config_channel_atten(config->channel, config->attenuation);
    } else {
        adc2_config_channel_atten((adc2_channel_t) config->channel, config->attenuation);
    }

    // Characterize ADC
    adc_chars = static_cast<esp_adc_cal_characteristics_t*>( calloc(1, sizeof(esp_adc_cal_characteristics_t)) );
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(config->unit, 
                                                            config->attenuation, 
                                                            config->width, 
                                                            vref, 
                                                            adc_chars);
}

uint32_t ADC::raw(uint8_t noOfSamples)
{
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < noOfSamples; i++) {
        if (config->unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t) config->channel);
        } else {
            int _raw;
            adc2_get_raw((adc2_channel_t) config->channel, config->width, &_raw);
            adc_reading += _raw;
        }
    }

    adc_reading /= noOfSamples;
    return esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
}