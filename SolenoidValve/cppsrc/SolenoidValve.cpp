#include "SolenoidValve.hpp"

SolenoidValve::SolenoidValve(string name, uint8_t pin_enable_, 
                    uint8_t t_pin_in1, uint8_t t_pin_in2, 
                    string nvs_ns) : 
                    name_(name), nvs_ns_(nvs_ns), 
                    m_pin_enable( (gpio_num_t) pin_enable_), 
                    m_pin_in1( (gpio_num_t) t_pin_in1), 
                    m_pin_in2( (gpio_num_t) t_pin_in2), 
                    valve_state_(false)
{
    esp_err_t err;
    gpio_set_direction(m_pin_enable, GPIO_MODE_OUTPUT);
    gpio_set_direction(m_pin_in1, GPIO_MODE_OUTPUT);
    gpio_set_direction(m_pin_in2, GPIO_MODE_OUTPUT);

    // check if key/value pair in NVS already exist
    handle_ = nvs::open_nvs_handle(nvs_ns_.c_str(), 
                                  NVS_READWRITE, 
                                  &err);

    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "open NVS");
    }

    err = handle_->get_item(name_.c_str(), valve_state_);

    switch(err)
    {
        case ESP_ERR_NVS_NOT_FOUND:
                err = handle_->set_item(name_.c_str(), valve_state_);
                if(err != ESP_OK)
                {
                    ESP_LOGE(TAG, "write valve_state init value to NVS");
                }
                handle_->commit();
                break;
        case ESP_OK:
                ESP_LOGI(TAG, "Valve state is %d", valve_state_);
                break;
        default:
                ESP_LOGE(TAG, "Reading value from NVS");
    }
    
    setValveState(valve_state_);
}

esp_err_t SolenoidValve::setValveState(uint8_t new_state)
{
    esp_err_t err;

    if(new_state != valve_state_)
    {
        valve_state_ = new_state;    
        err = handle_->set_item(name_.c_str(), valve_state_);
        if(err != ESP_OK)
        {
            ESP_LOGE(TAG, "write valve_state init value to NVS");
            return err;
        }
        handle_->commit();

        if(valve_state_ == true)
        {
            gpio_set_level(m_pin_in1, true);
            gpio_set_level(m_pin_in2, false);
        } else if(valve_state_ == false)
        {
            gpio_set_level(m_pin_in1, false);
            gpio_set_level(m_pin_in2, true);
        }

        gpio_set_level(m_pin_enable, true);
        vTaskDelay( SolenoidValve_SwitchTime_MS/ portTICK_PERIOD_MS);
        // gpio_set_level(m_pin_enable, false);
    } else {
        printf("nothing to do\n");
    }
    return ESP_OK;
}

void SolenoidValve::release()
{
    gpio_set_level(m_pin_enable, false);
}

esp_err_t SolenoidValve::toggleValveState()
{
    return setValveState(!valve_state_);
}
