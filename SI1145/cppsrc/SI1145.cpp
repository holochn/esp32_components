#include "SI1145.hpp"

static const char* TAG = "si1145";

SI1145::SI1145(gpio_num_t sda, gpio_num_t scl, uint32_t freq, uint8_t i2c_port, bool pullup_enable) 
        : I2cMasterNode(sda, scl, freq, i2c_port, pullup_enable)
{
    if(this->init() != ESP_OK)
    {
        ESP_LOGE(TAG, "init");
    }
}

esp_err_t SI1145::setParameter(uint8_t param, uint8_t val)
{
    esp_err_t err;
    uint8_t tmp;
    err = i2c_register_write(SI1145_ADDR, SI1145_REG_PARAM_WR, val);

    err = i2c_register_write(SI1145_ADDR, SI1145_REG_CMD, SI1145_PARAM_SET_PREFIX | param);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "Set parameter");
        return err;
    }

    tmp = i2c_register_read8(SI1145_ADDR, SI1145_REG_RESPONSE);
    if(tmp >= SI1145_CMD_ERR_INV)
    {
        ESP_LOGE(TAG, "error response");
        return ESP_ERR_INVALID_RESPONSE;
    }

    tmp = i2c_register_read8(SI1145_ADDR, SI1145_REG_PARAM_RD);
    if(tmp != val)
    {
        ESP_LOGE(TAG, "error writing value");
        return ESP_ERR_INVALID_RESPONSE;
    }

    return ESP_OK;
}

esp_err_t SI1145::recoverUVParam()
{
    // write at command register SI1145_REG_CMD
    // value SI1145_PARAM_RECOVER_UV

    return ESP_OK;
}

esp_err_t SI1145::setMode(SI1145_mode_t mode)
{
    esp_err_t err = ESP_ERR_NOT_SUPPORTED;
    if(mode == SI1145_MODE_AUTO)
    {

    } else if(mode == SI1145_MODE_FORCED_ALS)
    {
        err = i2c_register_write(SI1145_ADDR, SI1145_REG_CMD, SI1145_CMD_MODE_ALS_FORCE);
    } else if(mode == SI1145_MODE_FORCED_PS)
    {
        err = i2c_register_write(SI1145_ADDR, SI1145_REG_CMD, SI1145_CMD_MODE_PS_FORCE);
    } else if(mode == SI1145_MODE_FORCED_SINGLE)
    {
        err = i2c_register_write(SI1145_ADDR, SI1145_REG_CMD, SI1145_CMD_MODE_SNGL_FORCE);
    } else {
        return ESP_ERR_NOT_SUPPORTED;
    }

    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "set mode");
        return ESP_ERR_INVALID_STATE;
    }

    return ESP_OK;
}

esp_err_t SI1145::init()
{
    esp_err_t err; 
    uint8_t id;

    id = i2c_register_read8(SI1145_ADDR, SI1145_REG_PART_ID);
    if( id != 0x45 )
    {
        ESP_LOGE(TAG, "invalid version");
        return ESP_ERR_INVALID_VERSION;
    } 
    
    err = i2c_register_write(SI1145_ADDR, SI1145_REG_HW_KEY, SI1145_PARAM_INIT);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "write init value");
        return err;
    }

    return ESP_OK;
}

esp_err_t SI1145::enableUV()
{
    esp_err_t err;

    i2c_register_write(SI1145_ADDR, SI1145_REG_UCOEF0, SI1145_PARAM_UCOEF0_UV);
    i2c_register_write(SI1145_ADDR, SI1145_REG_UCOEF1, SI1145_PARAM_UCOEF1_UV);
    i2c_register_write(SI1145_ADDR, SI1145_REG_UCOEF2, SI1145_PARAM_UCOEF2_UV);
    i2c_register_write(SI1145_ADDR, SI1145_REG_UCOEF3, SI1145_PARAM_UCOEF3_UV);

    err = setParameter(SI1145_PARAM_CHLIST, SI1145_PARAM_EN_UV | SI1145_PARAM_EN_ALS_IR 
                                        | SI1145_PARAM_EN_ALS_VIS | SI1145_PARAM_EN_PS1);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "enable UV");
        return err;
    }
    return ESP_OK;
}

uint8_t SI1145::getID()
{
    return i2c_register_read8(SI1145_ADDR, SI1145_REG_PART_ID);
}

uint16_t SI1145::getUvIndex()
{
    
    return i2c_register_read16_lien(SI1145_ADDR, SI1145_REG_UV_INDEX0); 
    
    uint16_t result = 0;
    uint8_t tmp;
    tmp = i2c_register_read8(SI1145_ADDR, SI1145_REG_UV_INDEX1);
    printf("tmp %x\n", tmp);
    result = tmp;
    result <<= 8;
    tmp = i2c_register_read8(SI1145_ADDR, SI1145_REG_UV_INDEX0);
    printf("tmp %x\n", tmp);
    result |= tmp;
    printf("result %x\n", result);
    return result;
}