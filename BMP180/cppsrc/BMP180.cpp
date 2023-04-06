#include "BMP180.hpp"

static const char* TAG = "bmp180";

static int16_t  ac1 = 0;
static int16_t  ac2 = 0;
static int16_t  ac3 = 0;
static uint16_t ac4 = 0;
static uint16_t ac5 = 0;
static uint16_t ac6 = 0;
static int16_t  b1  = 0;
static int16_t  b2  = 0;
static int32_t  b3  = 0;
static uint32_t b4  = 0;
static int32_t  b5  = 0;
static int32_t  b6  = 0;
static int32_t  b7  = 0;
static int16_t  mb  = 0;
static int16_t  mc  = 0;
static int16_t  md  = 0;

BMP180::BMP180(gpio_num_t sda, gpio_num_t scl, uint32_t freq, uint8_t i2c_port, bool pullup_enable) 
        : I2cMasterNode(sda, scl, freq, i2c_port, pullup_enable)
{
    esp_err_t err;
    err = this->get_cal_data();
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "cal data: %x", err);
    }
}

esp_err_t BMP180::get_cal_data()
{
    
    ac1 |= (int16_t) i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_AC1_MSB);
    ac2 |= (int16_t) i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_AC2_MSB);
    ac3 |= (int16_t) i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_AC3_MSB);
    ac4 |= i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_AC4_MSB);
    ac5 |= i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_AC5_MSB);
    ac6 |= i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_AC6_MSB);
    b1  |= (int16_t) i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_B1_MSB );
    b2  |= (int16_t) i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_B2_MSB );
    mb  |= (int16_t) i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_MB_MSB );
    mc  |= (int16_t) i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_MC_MSB );
    md  |= (int16_t) i2c_register_read16(BMP180_ADDR, BMP180_CAL_PARAM_MD_MSB );

    ESP_LOGI(TAG, "ac1=%02x", ac1);
    ESP_LOGI(TAG, "ac2=%02x", ac2);
    ESP_LOGI(TAG, "ac3=%02x", ac3);
    ESP_LOGI(TAG, "ac4=%02x", ac4);
    ESP_LOGI(TAG, "ac5=%02x", ac5);
    ESP_LOGI(TAG, "ac6=%02x", ac6);
    ESP_LOGI(TAG, "b1 =%02x", b1);
    ESP_LOGI(TAG, "b2 =%02x", b2);
    ESP_LOGI(TAG, "mb =%02x", mb);
    ESP_LOGI(TAG, "mc =%02x", mc);
    ESP_LOGI(TAG, "md =%02x", md);

    if((ac1 == 0) || (ac2 == 0) || (ac3 == 0) || (ac4 == 0) || (ac5 == 0) || (ac6 == 0) || 
        (b1 == 0) || (b2 = 0) || (mb == 0) || (mc == 0) || (md == 0))
    {
        return 234;
    }

    if((ac1 == 0x7FFF) || (ac2 == 0x7FFF) || (ac3 == 0x7FFF) || (b1 == 0x7FFF) || 
        (b2 == 0x7FFF) || (mb == 0x7FFF) || (mc == 0x7FFF) || (md == 0x7FFF))
    {
        return 235;
    }
    if((ac4 == 0xFFFF) || (ac5 == 0xFFFF) || (ac6 == 0xFFFF))
    {
        return 236;
    }

    return ESP_OK;
}

void BMP180::delay()
{
    vTaskDelay(BMP180_MEAS_TIME_MS / portTICK_PERIOD_MS);
}

uint16_t BMP180::get_ut()
{
    i2c_register_write(BMP180_ADDR, BMP180_REG_CTRL_MEAS, BMP180_MEAS_TEMP);
    this->delay();

    return i2c_register_read16(BMP180_ADDR, BMP180_REG_OUT_MSB);
}

int32_t BMP180::get_up()
{
    int32_t result = 0;
    uint8_t msb = 0;
    uint8_t lsb = 0;
    uint8_t xlsb = 0;
    i2c_register_write(BMP180_ADDR, BMP180_REG_CTRL_MEAS, BMP180_MEAS_PRES | (BMP180_OVERSAMPLING << 6));
    this->delay();

    msb  |= i2c_register_read8(BMP180_ADDR, BMP180_REG_OUT_MSB);
    lsb  |= i2c_register_read8(BMP180_ADDR, BMP180_REG_OUT_LSB);
    xlsb |= i2c_register_read8(BMP180_ADDR, BMP180_REG_OUT_XLSB);

    result = ((int32_t) (msb << 16)) | ((int32_t) (lsb << 8)) | ((int32_t) xlsb);
    result >>= (8-BMP180_OVERSAMPLING);

    return result;
}

int32_t BMP180::get_temperature()
{
    int32_t ut = 0;
    int32_t x1 = 0;
    int32_t x2 = 0;

    ut |= (int32_t) this->get_ut();
    
    x1 = (ut - (int32_t) ac6) * ((int32_t) ac5);
    x1 >>= 15;
    
    if((x1 == 0) || (md == 0))
    {
        ESP_LOGI(TAG, "Division by zero x1=%d\tmd=%d", x1, md);
        return -1;
    }

    x2 = ((int32_t) mc << 11) / ((int32_t) (x1 + ((int32_t) md)));
    b5 = x1 + x2;
    
    return ((b5 + 8) >> 4);
}

int32_t BMP180::get_pressure()
{
    int32_t result = 0;
    int32_t x1 = 0;
    int32_t x2 = 0;
    int32_t x3 = 0;

    this->get_ut();
    uint32_t up = this->get_up();

    b6 = b5 - 4000;
    x1 = (b6*b6) >> 12;
    x1 *= (int32_t) b2;
    x1 >>= 11;
    x2 = ((int32_t) ac2)*b6;
    x2 >>= 11;
    x3 = x1 + x2;
    b3 = ((((int32_t) ac1) * 4 + x3)) << (BMP180_OVERSAMPLING + 2) / 4;
    
    x1 = (((int32_t) ac3) * b6) >> 13;
    x2 = (b6*b6) >> 12;
    x2 *= (int32_t) b1;
    x2 >>= 16;
    x3 = ((x1 + x2) + 2) / 2;
    b4 = (((int32_t) ac4) * (x3 + 32768)) >> 15;
    b7 = ((int32_t) (up - b3)) * (50000 >> BMP180_OVERSAMPLING);
    
    if(b4 == 0)
    {
        ESP_LOGI(TAG, "Division by zero (b4)");
        return -1;
    }

    if(b7 < 0x80000000)
    {
        result = (b7 * 2) / b4;
    } else {
        result = (b7 / b4) * 2;
    }
    x1 = (result >> 8);
    x1 *= x1;
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * result) >> 16;
    result += (x1 + x2 + 3791) / 16;

    return result;
}

uint8_t BMP180::getID()
{
    return i2c_register_read8(BMP180_ADDR, BMP180_REG_ID);
}

void BMP180::reset()
{
    i2c_register_write(BMP180_ADDR, BMP180_REG_SOFT_RST, BMP180_PARAM_RESET);
}