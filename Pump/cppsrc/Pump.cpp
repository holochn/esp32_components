#include "Pump.hpp"

Pump::Pump(uint8_t gpio_num)
{
    if(gpio_num >= GPIO_NUM_MAX)
    {
        ESP_LOGE(TAG, "Invalid GPIO argument");
    }

    pin = (gpio_num_t) gpio_num;

    gpio_set_direction(pin, GPIO_MODE_OUTPUT);

    ledc_timer.duty_resolution    = LEDC_TIMER_8_BIT;
    ledc_timer.freq_hz            = 30;
    ledc_timer.speed_mode         = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num          = LEDC_TIMER_1;
    ledc_timer.clk_cfg            = LEDC_AUTO_CLK;

    ledc_timer_config(&ledc_timer);

    ledc_channel.channel         = LEDC_CHANNEL_2;
    ledc_channel.duty            = 0;
    ledc_channel.gpio_num        = pin;
    ledc_channel.speed_mode      = LEDC_LOW_SPEED_MODE;
    ledc_channel.hpoint          = 0;
    ledc_channel.intr_type       = LEDC_INTR_DISABLE;
    ledc_channel.timer_sel       = LEDC_TIMER_1;

    ledc_channel_config(&ledc_channel);
    ledc_stop(ledc_channel.speed_mode, ledc_channel.channel, 0);
}

void Pump::start(const uint8_t pwm_duty_cycle)
{
    uint8_t pwm = ((1<<ledc_timer.duty_resolution)-1) * pwm_duty_cycle / 100;
    if( pwm != ledc_get_duty(ledc_channel.speed_mode, ledc_channel.channel) )
    {  
        printf("New duty cycle: %d\n", pwm);

        ledc_set_duty(ledc_channel.speed_mode, 
                    ledc_channel.channel,
                    pwm);
        ledc_update_duty(ledc_channel.speed_mode, 
                    ledc_channel.channel);
    }
    ledc_timer_resume(ledc_timer.speed_mode, 
                    ledc_timer.timer_num);
}

void Pump::set_duty(const uint8_t pwm_duty_cycle)
{
    uint8_t pwm = ((1<<ledc_timer.duty_resolution)-1) * pwm_duty_cycle / 100;

    ledc_set_duty(ledc_channel.speed_mode, 
                    ledc_channel.channel,
                    pwm);
    ledc_update_duty(ledc_channel.speed_mode, 
                    ledc_channel.channel);
    
    ledc_timer_resume(ledc_timer.speed_mode, 
                    ledc_timer.timer_num);
}

void Pump::stop()
{
    set_duty(0);
    // ledc_timer_pause(ledc_timer.speed_mode,
    //                ledc_timer.timer_num);
}