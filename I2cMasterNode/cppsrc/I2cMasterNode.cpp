#include "I2cMasterNode.hpp"

static const char* TAG = "I2cMasterNode";

I2cMasterNode::I2cMasterNode(gpio_num_t sda, gpio_num_t scl, uint32_t freq, uint8_t i2c_port, bool pullup_enable)
{
    this->i2c_sda   = sda;
    this->i2c_scl   = scl;
    this->i2c_freq  = freq;
    this->i2c_pullup_enable = pullup_enable;
    this->i2c_config.mode          = I2C_MODE_MASTER;
    this->i2c_config.sda_io_num    = sda;
    this->i2c_config.scl_io_num    = scl;
    this->i2c_config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    this->i2c_config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    if(pullup_enable == false)
    {
        this->i2c_config.sda_pullup_en = GPIO_PULLUP_DISABLE;
        this->i2c_config.scl_pullup_en = GPIO_PULLUP_DISABLE;
    }
    this->i2c_config.master.clk_speed = freq;
    
    if(I2C_NUM_MAX <= (i2c_port_t) i2c_port)
    {
        ESP_LOGE(TAG, "Invalid port");
    }

    this->port = (i2c_port_t) i2c_port;
}

esp_err_t I2cMasterNode::i2c_master_initialize()
{
    i2c_config.mode             = I2C_MODE_MASTER;
    i2c_config.sda_io_num       = i2c_sda;
    i2c_config.scl_io_num       = i2c_scl;
    i2c_config.sda_pullup_en    = i2c_pullup_enable;
    i2c_config.scl_pullup_en    = i2c_pullup_enable;
    i2c_config.clk_flags        = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
    i2c_config.master.clk_speed = i2c_freq;
   
    return i2c_param_config(port, &i2c_config);
}

uint8_t I2cMasterNode::i2c_register_read8(uint8_t addr, uint8_t reg)
{
    uint8_t data = 0;
    esp_err_t err;

    if(i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0) != ESP_OK)
    {
        ESP_LOGE(TAG, "driver for read8");
    }
    if(i2c_master_initialize() != ESP_OK)
    {
        ESP_LOGE(TAG, "read8: Master init");
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr << 1 | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, I2C_ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr << 1 | I2C_MASTER_READ, I2C_ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_RATE_MS);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "read8 from I2C: %x", err);
    }

    i2c_cmd_link_delete(cmd);   
    i2c_driver_delete(port);

    return data;
}

uint16_t I2cMasterNode::i2c_register_read16(uint8_t addr, uint8_t reg)
{
    uint16_t result = 0;
    uint8_t tmp = 0;

    tmp = i2c_register_read8(addr, reg);
    result = tmp;
    result <<= 8;
    ++reg;
    result |= i2c_register_read8(addr, reg);

    return result;
}

uint16_t I2cMasterNode::i2c_register_read16_lien(uint8_t addr, uint8_t reg)
{
    uint16_t result = 0;
    uint16_t tmp = 0;

    result = i2c_register_read8(addr, reg);
    ++reg;
    tmp = i2c_register_read8(addr, reg);
    tmp <<= 8;
    result |= tmp;

    return result;
}

esp_err_t I2cMasterNode::i2c_register_write(uint8_t addr, uint8_t reg, uint8_t reg_val)
{
    esp_err_t err;
    err = i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "driver for write");
        return err;
    }

    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_initialize();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr << 1 | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, reg_val, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    
    err = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_RATE_MS);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "write to I2C");
        return err;
    }

    i2c_cmd_link_delete(cmd);   
    i2c_driver_delete(port);
    
    return ESP_OK;
}