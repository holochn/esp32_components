#include "FrequencySensor.hpp"

static const char* TAG = "FrequencySensor";

static void IRAM_ATTR pcnt_event_handler(void*  arg)
{
    frequencysensor_evt_handl_arg_t *tmp = (frequencysensor_evt_handl_arg_t*) arg;
    FrequencySensor* fs = (FrequencySensor*) tmp->ptrFS;
    portENTER_CRITICAL_ISR(tmp->mux);
    fs->incOverflowCounter();
    PCNT.int_clr.val = BIT(fs->getPcntUnit());
    portEXIT_CRITICAL_ISR(tmp->mux);
}

void FrequencySensor::pcnt_get_counter(void* p)
{
    uint16_t result = 0;
    FrequencySensor* fs = (FrequencySensor*) p;
    pcnt_unit_t unit = fs->getPcntUnit();

    pcnt_counter_pause(unit);
    pcnt_get_counter_value(unit, (int16_t*) &result);
    fs->calcFrequency(result);
    fs->setOverflowCounter(0);
    pcnt_counter_clear(unit);
    pcnt_counter_resume(unit);
}

FrequencySensor::FrequencySensor(pcnt_config_t *config, 
                    uint32_t min_val, 
                    uint32_t max_val,
                    uint64_t sample_time)
{
    timer_mux   = portMUX_INITIALIZER_UNLOCKED;
    pcnt_config = config;
    sensor_min  = min_val;
    sensor_max  = max_val;
    frequency   = 0;
    overflow_count = 0;
    sensor_sample_time = sample_time;

    evnt_handle_arg.ptrFS = (void*) this;
    evnt_handle_arg.mux   = &timer_mux;
    
    timer_args.name             = "one shot timer";
    timer_args.callback         = FrequencySensor::pcnt_get_counter;
    timer_args.arg              = (void*) this;
    
    if(esp_timer_create(&timer_args, &timer_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "timer create");
    }

    // ios
    gpio_set_direction((gpio_num_t) pcnt_config->pulse_gpio_num, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t) pcnt_config->ctrl_gpio_num, GPIO_MODE_INPUT);

    // configure the pcnt
    pcnt_unit_config(pcnt_config);
    
    if(pcnt_event_enable(pcnt_config->unit, PCNT_EVT_H_LIM) != ESP_OK)
    {
        ESP_LOGE(TAG, "enable event");
        esp_restart();
    }
    pcnt_counter_pause(pcnt_config->unit);
    pcnt_counter_clear(pcnt_config->unit);

    if(pcnt_isr_register(pcnt_event_handler, (void*) &evnt_handle_arg, 0, NULL) != ESP_OK)
    {
        ESP_LOGE(TAG, "isr register");
        esp_restart();
    }
    
    if(pcnt_intr_enable(pcnt_config->unit) != ESP_OK)
    {
        ESP_LOGE(TAG, "intr enable");
        esp_restart();
    }
}

void FrequencySensor::calcFrequency(const uint16_t pcnt_counter)
{
    frequency = (double) (pcnt_counter + (overflow_count * getHlim() / 2));
}

double FrequencySensor::getFrequency() const
{
    return frequency;
}

void FrequencySensor::incOverflowCounter()
{
    overflow_count++;
}

uint16_t FrequencySensor::getOverflowCounter() const
{
    return overflow_count;
}

void FrequencySensor::setOverflowCounter(uint16_t val)
{
    overflow_count = val;
}

int16_t FrequencySensor::getHlim() const
{
    return pcnt_config->counter_h_lim;
}

pcnt_unit_t FrequencySensor::getPcntUnit() const
{
    return pcnt_config->unit;
}