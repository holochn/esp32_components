#include "DHT22.hpp"

DHT22::DHT22(uint8_t t_data_pin) : 
    m_data_pin{ gpio_num_t(t_data_pin)}
{
    gpio_set_direction(m_data_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(m_data_pin, true);
}

DHT22::~DHT22() {}

void DHT22::read()
{
    uint8_t cnt =0;
    
    // start sequence by pulling data_pin to LOW for 1 ms
    gpio_set_direction(m_data_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(m_data_pin, false);
    for (int i = 0; i < 1200; ++i)
    {
        ets_delay_us(1);
    }
    

    gpio_set_direction(m_data_pin, GPIO_MODE_INPUT);
    // 20-40 uSec for the DTH to signal operation mode
    cnt = expectSignalCycles(false, DHT22_MCU_PULLUP_START_TIME_US, 2);
    if( cnt == DHT22_TIMEOUT ) 
    {
        printf("TIMEOUT waiting for sensor response\n");
        return;
    }

    // wait for sensor to answer
    portMUX_TYPE myMux = portMUX_INITIALIZER_UNLOCKED;
    taskENTER_CRITICAL(&myMux);
    
    cnt = expectSignalCycles(true, DHT22_SENS_SIGNAL_START_TIME_US, 2);
    if( cnt == DHT22_TIMEOUT )
    {
        taskEXIT_CRITICAL(&myMux);
        printf("TIMEOUT start sequence low\n");
        return;
    }

    cnt = expectSignalCycles(false, DHT22_SENS_SIGNAL_START_TIME_US, 2);
    if( cnt == DHT22_TIMEOUT )
    {
        taskEXIT_CRITICAL(&myMux);
        printf("TIMEOUT start sequence high\n");
        return;
    }

    uint8_t byteIndex = 0;
    int8_t bitIndex  = 7;
    for (int i = 0; i < 40; ++i)
    {
        cnt=0;
        expectSignalCycles(true, DHT22_SENS_SIGNAL_PREBIT_TIME_US, 2);
        cnt = expectSignalCycles(false, 75, 2);
    
        if(cnt >= DHT22_SENS_SIGNAL_ONE_TIME_US)
        {
            data[byteIndex] |= (1 << bitIndex);
        }

        --bitIndex; 
        if(bitIndex < 0)
        {
            bitIndex=7;
            ++byteIndex;
        }
    }
    
    taskEXIT_CRITICAL(&myMux);

    if( data[4] != ((data[0]+data[1]+data[2]+data[3]) & 0xFF) )
    {
        printf("Checksum Error\n");
    }
}

int8_t DHT22::expectSignalCycles(bool expected_value, uint8_t max, uint8_t intervall)
{
    for (int i = 0; i < max; i+=intervall)
    {
        if( gpio_get_level(m_data_pin) == expected_value )
        {
            return i;
        }
        ets_delay_us(intervall);
    }
    
    return -1;
}

float DHT22::getHumidity()
{
    float humidity=0;

    uint16_t upper = (data[0] << 8);
    upper |= data[1];
    
    humidity = upper;
    humidity *= 0.1f;

    return humidity;
}

float DHT22::getTemperature()
{
    float temperature=0;
    uint16_t upper = (data[2] & 0x7F);
    upper <<= 8;
    upper |= data[3];

    temperature = static_cast<float>(upper);
    temperature /= 10;
    if( data[15] & 0x80)
    {
        temperature *= -1.0;
    }

    return temperature;
}