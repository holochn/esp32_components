#include "WaterLevelSensor.hpp"

static const char* WATERSENS_MIN = "watersens_min";
static const char* WATERSENS_MAX = "watersens_max";
static const char* TAG = "WaterLevelSensor";

WaterLevelSensor::WaterLevelSensor(pcnt_config_t *config, 
                    uint32_t min_val, uint32_t max_val, uint64_t sample_time) 
                    : FrequencySensor(config, 
                    min_val, max_val, sample_time) 
{
    storeValsToNVS = false;
}

int8_t WaterLevelSensor::getWaterLevel()
{
    double result = 0;
    esp_err_t err = ESP_OK;
    
    pcnt_counter_clear(pcnt_config->unit);
    esp_timer_start_once(timer_handle, sensor_sample_time);

    vTaskDelay( (sensor_sample_time + 1)/portTICK_PERIOD_MS);

    result =  frequency * (1000/sensor_sample_time);
    if(result > 0)
    {
        if(result > sensor_max)
        {
            sensor_max = result;
            if( storeValsToNVS == true )
            {
                std::shared_ptr<nvs::NVSHandle> nvs_handle = nvs::open_nvs_handle(WATERSENS_MAX, 
                                    NVS_READWRITE, 
                                    &err);
                err = nvs_handle->set_item(WATERSENS_MAX, sensor_max);
                if(err != ESP_OK)
                {
                    ESP_LOGE(TAG, "%s: write to NVS", __FUNCTION__);
                }
                nvs_handle->commit();
            }
        }

        if(result < sensor_min)
        {
            sensor_min = result;

            if( storeValsToNVS == true )
            {
                std::shared_ptr<nvs::NVSHandle> nvs_handle = nvs::open_nvs_handle(WATERSENS_MIN, 
                                    NVS_READWRITE, 
                                    &err);
                err = nvs_handle->set_item(WATERSENS_MIN, sensor_min);
                if(err != ESP_OK)
                {
                    ESP_LOGE(TAG, "%s: write to NVS", __FUNCTION__);
                }
                nvs_handle->commit();
            }
        }
    }

    result = ((100.0 - 0.0) / (sensor_max - sensor_min)) * (result - sensor_min);
    return 100 - (int8_t) result;
}

void WaterLevelSensor::useNVS(bool bUseNVS)
{
    esp_err_t err = ESP_OK;
    storeValsToNVS = bUseNVS;

    if(storeValsToNVS == true)
    {
        std::shared_ptr<nvs::NVSHandle> nvs_handle = nvs::open_nvs_handle(WATERSENS_MIN, 
                                    NVS_READWRITE, 
                                    &err);
        err = nvs_handle->get_item(WATERSENS_MIN, sensor_min);
        if(err != ESP_OK)
        {
            nvs_handle->set_item(WATERSENS_MIN, sensor_min);
            ESP_LOGE(TAG, "%s: write to NVS", __FUNCTION__);
        }

        nvs_handle = nvs::open_nvs_handle(WATERSENS_MAX, 
                                    NVS_READWRITE, 
                                    &err);
        err = nvs_handle->get_item(WATERSENS_MAX, sensor_max);
        if(err != ESP_OK)
        {
            nvs_handle->set_item(WATERSENS_MAX, sensor_max);
            ESP_LOGE(TAG, "%s: write to NVS", __FUNCTION__);
        }
    }
}
