#include "AdcSensor.hpp"

static const char* TAG = "AdcSensor";

AdcSensor::AdcSensor(adc_unit_t adc_unit, adc1_channel_t adc_channel, adc_atten_t adc_atten, 
                adc_bits_width_t adc_width, uint8_t num_samples, uint32_t adc_default_vref, 
                uint32_t min_value, uint32_t max_value) : storeValsToNVS{0}, cal_count{0}
{
    if((adc_unit <= 0) || (adc_unit >= ADC_UNIT_MAX) )
    {
        ESP_LOGE(TAG, "invalid adc unit");
    }
    unit = adc_unit;
    channel = adc_channel;
    if((adc_atten <= 0) || (adc_atten >= ADC_ATTEN_MAX) )
    {
        ESP_LOGE(TAG, "invalid adc attentuation");
    }
    atten = adc_atten;
    if(adc_width >= ADC_WIDTH_MAX)
    {
        ESP_LOGE(TAG, "invalid adc width");
    }
    width = adc_width;
    adc1_config_width(width);

    if(num_samples == 0)
    {
        ESP_LOGE(TAG, "invalid number fo samples");
        samples = 1;
    } else {
        samples = num_samples;
    }
    default_vref = adc_default_vref;
    this->sensor_min = min_value;
    this->sensor_max = max_value;

    adc1_config_channel_atten(channel, atten);
    adc_chars= (esp_adc_cal_characteristics_t *) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, atten, width, default_vref, adc_chars);
}


uint32_t AdcSensor::getVoltage()
{
    int32_t reading = 0;
    uint32_t result  = 0;

    for(int i = 0; i < samples; i++)
    {
        reading = getVoltageRaw();
        if(reading >= 0)
        {
            if( reading < sensor_min )
            {
                reading = sensor_min;
            }
            if( reading > sensor_max)
            {
                reading = sensor_max;
            }
            result += reading;
        }
    }
    result /= samples;

    return result;
}

int32_t AdcSensor::getVoltageRaw()
{
    int32_t reading = 0;
    reading = adc1_get_raw(channel);
    return esp_adc_cal_raw_to_voltage(reading, adc_chars);
}

uint8_t AdcSensor::getValue()
{
    esp_err_t err = ESP_OK;
    nvs_handle_t nvs_handle;
    int32_t reading = 0;
    float tmp = 0;
    uint8_t result = 0;
    int16_t sens_delta_max = 0;
    int16_t reading_norm = 0;

    
    if( storeValsToNVS == true )
    {
        err = nvs_open(nvs_ns.c_str(), NVS_READWRITE, &nvs_handle);
        err = nvs_get_u32(nvs_handle, nvs_ns_min.c_str(), &sensor_min);
        err = nvs_get_u32(nvs_handle, nvs_ns_max.c_str(), &sensor_max);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        
        if(err != ESP_OK)
        {
            ESP_LOGE(TAG, "%s: Reading from NVS", __FUNCTION__);
            return 255;
        }
    }
    
    if(sensor_min == sensor_max)
    {
        sensor_min = sensor_max - 3;
    }
    
    reading = getVoltage();
    cal_count++;
    if( storeValsToNVS == true )
    {
        sensor_learn_values(reading);
        if(cal_count % 430 == 0)
        {
            calibrate();
        }
    }
    
    sens_delta_max = sensor_max - sensor_min;
    reading_norm   = reading - sensor_min;
    tmp = (100.0 * reading_norm) / sens_delta_max;
    tmp = 100.0 - tmp;
    result = (uint8_t) tmp;
    if(result > 100)
    {
        result = 100;
    }
    return (uint8_t) result;
}

uint32_t AdcSensor::getMin() const
{
    return sensor_min;
}

uint32_t AdcSensor::getMax() const
{
    return sensor_max;
}

void AdcSensor::useNvs(const char* t_nvs_ns)
{
    esp_err_t err = ESP_OK;
    storeValsToNVS = true;
    nvs_ns = t_nvs_ns;
    nvs_ns_min = nvs_ns;
    nvs_ns_min.append("_min");
    nvs_ns_max = nvs_ns;
    nvs_ns_max.append("_max");

    nvs_handle_t nvs_handle;

    err = nvs_open(nvs_ns.c_str(), NVS_READWRITE, &nvs_handle);
    err = nvs_get_u32(nvs_handle, nvs_ns_min.c_str(), &sensor_min);
    if(err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = nvs_set_u32(nvs_handle, nvs_ns_min.c_str(), ADC_SENS_DEFAULT);
        nvs_commit(nvs_handle);
    }
    
    err = nvs_get_u32(nvs_handle, nvs_ns_max.c_str(), &sensor_max);
    if(err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = nvs_set_u32(nvs_handle, nvs_ns_max.c_str(), 0x0);
        nvs_commit(nvs_handle);
    }

    nvs_close(nvs_handle);
}

void AdcSensor::resetNvs()
{
    esp_err_t err = ESP_OK;
    uint32_t min_t;
    uint32_t max_t;
    nvs_handle_t nvs_handle;

    err = nvs_open(nvs_ns.c_str(), NVS_READWRITE, &nvs_handle);
    err = nvs_set_u32(nvs_handle, nvs_ns_min.c_str(), ADC_SENS_DEFAULT);
    err = nvs_set_u32(nvs_handle, nvs_ns_max.c_str(), 0x0);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    std::shared_ptr<nvs::NVSHandle> nvs_hndl = nvs::open_nvs_handle(nvs_ns.c_str(), 
                                    NVS_READWRITE, 
                                    &err);

    err = nvs_hndl->get_item(nvs_ns_min.c_str(), min_t);
    err = nvs_hndl->get_item(nvs_ns_max.c_str(), max_t);
}


void AdcSensor::sensor_learn_values(uint32_t result)
{
    nvs_handle_t nvs_handle;

    if( (result < sensor_min) && (result != sensor_max) )
    {
        sensor_min = result;

        if( storeValsToNVS == true )
        {
            nvs_open(nvs_ns.c_str(), NVS_READWRITE, &nvs_handle);
            nvs_set_u32(nvs_handle, nvs_ns_min.c_str(), result);
            nvs_commit(nvs_handle);
            nvs_close(nvs_handle);
        }
    }

    if((result > sensor_max) && (result != sensor_min))
    {
        sensor_max = result;
        if( storeValsToNVS == true )
        {
            nvs_open(nvs_ns.c_str(), NVS_READWRITE, &nvs_handle);
            nvs_set_u32(nvs_handle, nvs_ns_max.c_str(), result);
            nvs_commit(nvs_handle);
            nvs_close(nvs_handle);
        }
    }
}

void AdcSensor::calibrate()
{
    nvs_handle_t nvs_handle;

    sensor_min += 100;
    sensor_max -= 100;

    if( storeValsToNVS == true )
    {
        nvs_open(nvs_ns.c_str(), NVS_READWRITE, &nvs_handle);
        nvs_set_u32(nvs_handle, nvs_ns_max.c_str(), sensor_max);
        nvs_set_u32(nvs_handle, nvs_ns_min.c_str(), sensor_min);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    }
}