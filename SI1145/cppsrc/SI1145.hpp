#ifndef SI1145_HPP
#define SI1145_HPP

#include "I2cMasterNode.hpp"

const uint8_t SI1145_ADDR               = 0x60;

const uint8_t SI1145_REG_PART_ID        = 0x00;
const uint8_t SI1145_REG_REV_ID         = 0x01;
const uint8_t SI1145_REG_SEQ_ID         = 0x02;
const uint8_t SI1145_REG_INT_CFG        = 0x03;
const uint8_t SI1145_REG_IRQ_ENABLE     = 0x04;
const uint8_t     SI1145_REG_ALS_IE     = 0x01;
const uint8_t     SI1145_REG_PS1_IE     = 0x04;
const uint8_t     SI1145_REG_PS2_IE     = 0x08;
const uint8_t     SI1145_REG_PS3_IE     = 0x10;
const uint8_t SI1145_REG_HW_KEY         = 0x07;
const uint8_t SI1145_REG_MEAS_RATE0     = 0x08;
const uint8_t SI1145_REG_MEAS_RATE1     = 0x09;
const uint8_t SI1145_REG_PS_RATE        = 0x0A;
const uint8_t SI1145_REG_UCOEF0         = 0x13;
const uint8_t SI1145_REG_UCOEF1         = 0x14;
const uint8_t SI1145_REG_UCOEF2         = 0x15;
const uint8_t SI1145_REG_UCOEF3         = 0x16;
const uint8_t SI1145_REG_PARAM_WR       = 0x17;
const uint8_t SI1145_REG_CMD            = 0x18;
const uint8_t SI1145_REG_RESPONSE       = 0x20;
const uint8_t SI1145_REG_IRQ_STATUS     = 0x21;
const uint8_t     SI1145_REG_ALS_INT    = 0x03;
const uint8_t     SI1145_REG_PS1_INT    = 0x04;
const uint8_t     SI1145_REG_PS2_INT    = 0x08;
const uint8_t     SI1145_REG_PS3_INT    = 0x10;
const uint8_t     SI1145_REG_CMD_INT    = 0x20;
const uint8_t SI1145_REG_ALS_VIS_DATA0  = 0x22;
const uint8_t SI1145_REG_ALS_VIS_DATA1  = 0x23;
const uint8_t SI1145_REG_ALS_IR_DATA0   = 0x24;
const uint8_t SI1145_REG_ALS_IR_DATA1   = 0x25;
const uint8_t SI1145_REG_PS1_DATA0      = 0x26;
const uint8_t SI1145_REG_PS1_DATA1      = 0x27;
const uint8_t SI1145_REG_PS2_DATA0      = 0x28;
const uint8_t SI1145_REG_PS2_DATA1      = 0x29;
const uint8_t SI1145_REG_PS3_DATA0      = 0x2A;
const uint8_t SI1145_REG_PS3_DATA1      = 0x2B;
const uint8_t SI1145_REG_UV_INDEX0      = 0x2C;
const uint8_t SI1145_REG_UV_INDEX1      = 0x2D;
const uint8_t SI1145_REG_PARAM_RD       = 0x2E;
const uint8_t SI1145_REG_CHIP_STAT      = 0x30;
const uint8_t     SI1145_REG_SLEEP      = 0x01;
const uint8_t     SI1145_REG_SUSPEND    = 0x02;
const uint8_t     SI1145_REG_RUNNING    = 0x04;

const uint8_t SI1145_CMD_MODE_PS_FORCE  = 0x05;
const uint8_t SI1145_CMD_MODE_ALS_FORCE = 0x06;
const uint8_t SI1145_CMD_MODE_SNGL_FORCE= 0x07;
const uint8_t SI1145_CMD_ERR_INV        = 0x80;
const uint8_t SI1145_CMD_ERR_PS1        = 0x88;
const uint8_t SI1145_CMD_ERR_PS2        = 0x89;
const uint8_t SI1145_CMD_ERR_PS3        = 0x8A;
const uint8_t SI1145_CMD_ERR_ALS_VIS    = 0x8C;
const uint8_t SI1145_CMD_ERR_ALS_IR     = 0x8D;
const uint8_t SI1145_CMD_ERR_AUX        = 0x8E;

const uint8_t SI1145_PARAM_I2C_ADDR     = 0x00;
const uint8_t SI1145_PARAM_CHLIST       = 0x01;
const uint8_t     SI1145_PARAM_EN_PS1   = 0x01;
const uint8_t     SI1145_PARAM_EN_PS2   = 0x02;
const uint8_t     SI1145_PARAM_EN_PS3   = 0x04;
const uint8_t     SI1145_PARAM_EN_ALS_VIS = 0x10;
const uint8_t     SI1145_PARAM_EN_ALS_IR= 0x20;
const uint8_t     SI1145_PARAM_EN_AUX   = 0x40;
const uint8_t     SI1145_PARAM_EN_UV    = 0x80;

const uint8_t SI1145_PARAM_UCOEF0_UV    = 0x29;
const uint8_t SI1145_PARAM_UCOEF1_UV    = 0x89;
const uint8_t SI1145_PARAM_UCOEF2_UV    = 0x02;
const uint8_t SI1145_PARAM_UCOEF3_UV    = 0x00;
const uint8_t SI1145_PARAM_RECOVER_UV   = 0x12;
const uint8_t SI1145_PARAM_INIT         = 0x17;
const uint8_t SI1145_PARAM_SET_PREFIX   = 0xA0;
const uint8_t SI1145_UV_MEAS_DELAY_MS   = 2;

typedef enum 
{
    SI1145_MODE_FORCED_ALS = 0,
    SI1145_MODE_FORCED_PS,
    SI1145_MODE_FORCED_SINGLE,
    SI1145_MODE_AUTO
} SI1145_mode_t;

class SI1145 : public I2cMasterNode 
{
    public:
    SI1145(gpio_num_t sda, gpio_num_t scl, uint32_t freq, uint8_t i2c_port=0, bool pullup_enable=true);
    esp_err_t enableUV();
    esp_err_t recoverUVParam();
    esp_err_t setMode(SI1145_mode_t mode);
    uint8_t getID();
    uint16_t getUvIndex();

    private:
    esp_err_t init();
    esp_err_t setParameter(uint8_t param, uint8_t val);
};
#endif // SI1145_HPP