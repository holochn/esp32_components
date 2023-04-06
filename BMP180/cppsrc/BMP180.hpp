#ifndef BMP180_HPP
#define BMP180_HPP

#include "I2cMasterNode.hpp"

#define BMP180_CAL_PARAM_AC1_MSB 0xAA
#define BMP180_CAL_PARAM_AC1_LSB 0xAB
#define BMP180_CAL_PARAM_AC2_MSB 0xAC 
#define BMP180_CAL_PARAM_AC2_LSB 0xAD
#define BMP180_CAL_PARAM_AC3_MSB 0xAE
#define BMP180_CAL_PARAM_AC3_LSB 0xAF
#define BMP180_CAL_PARAM_AC4_MSB 0xB0
#define BMP180_CAL_PARAM_AC4_LSB 0xB1
#define BMP180_CAL_PARAM_AC5_MSB 0xB2
#define BMP180_CAL_PARAM_AC5_LSB 0xB3
#define BMP180_CAL_PARAM_AC6_MSB 0xB4
#define BMP180_CAL_PARAM_AC6_LSB 0xB5
#define BMP180_CAL_PARAM_B1_MSB  0xB6
#define BMP180_CAL_PARAM_B1_LSB  0xB7
#define BMP180_CAL_PARAM_B2_MSB  0xB8
#define BMP180_CAL_PARAM_B2_LSB  0xB9
#define BMP180_CAL_PARAM_MB_MSB  0xBA
#define BMP180_CAL_PARAM_MB_LSB  0xBB
#define BMP180_CAL_PARAM_MC_MSB  0xBC
#define BMP180_CAL_PARAM_MC_LSB  0xBD
#define BMP180_CAL_PARAM_MD_MSB  0xBE
#define BMP180_CAL_PARAM_MD_LSB  0xBF

const uint8_t BMP180_ADDR           = 0x77;
const uint8_t BMP180_PROM_SADDR     = 0xAA;
const uint8_t BMP180_REG_ID         = 0xD0;
const uint8_t BMP180_REG_VERSION    = 0xD1;
const uint8_t BMP180_REG_SOFT_RST   = 0xE0;
const uint8_t BMP180_REG_CTRL_MEAS  = 0xF4;
const uint8_t BMP180_REG_OUT_MSB    = 0xF6;
const uint8_t BMP180_REG_OUT_LSB    = 0xF7;
const uint8_t BMP180_REG_OUT_XLSB   = 0xF8;
const uint8_t BMP180_MEAS_TEMP      = 0x2E;
const uint8_t BMP180_MEAS_PRES      = 0x34;
const uint8_t BMP180_PARAM_RESET    = 0xB6;
const uint8_t BMP180_PROM_SIZE      = 22;
const uint8_t BMP180_OVERSAMPLING   = 1;        // 0 = ultra low power | 1 = standard | 2 = high resolution | 3 = ultra high resolution 
const uint8_t BMP180_MEAS_TIME_MS   = 10;

/**
 * @brief: Implementation class of Bosch BMP180 temperature and pressure sensor
 * 
 */ 

class BMP180 : public I2cMasterNode 
{
    public:
    /**
     * @brief: Constructor passes arguments to constructor of base class I2CmasterNode
     * @param: 
     *          sda The GPIO of sda pin
     *          scl The GPIO of scl pin
     *          freq The I2C frequency
     *          i2c_port the I2C port. Default value is 0
     *          pullup_enable Flag to enable/disable pul up ESP's resistors for sda and scl. Default value is true
     */
    BMP180(gpio_num_t sda, gpio_num_t scl, uint32_t freq, uint8_t i2c_port=0, bool pullup_enable=true);
    
    /**
     * @brief: Returns the temperature reading of BMP180 
     *         in degree Celsius with a resolution of 0.1 degree. 
     *         Calling function must divide return value by 10
     * @param: 
     * @return: Temperature reading as int32_t in degree Celsius
     */
    int32_t get_temperature();
    
    /**
     * @brief: Returns the air pressure reading of BMP180 
     *         in Pascal
     * @param: 
     * @return: air pressure reading as int32_t in Pascal
     */
    int32_t get_pressure();

    /**
     * @brief: Returns the chip ID of BMP180.
     *         Can be used to test I2C communication
     * @param: 
     * @return: Chip ID as uint8_t
     */
    uint8_t getID();

    /**
     * @brief: Triggers a soft reset fo BMP180
     * @param: 
     * @return: 
     */
    void reset();

    /**
     * @brief: Makes the BMP180 read its calibration values
     * @param: 
     * @return: 
     *          ESP_OK on success
     *          234 - If any of the calibration data is zero
     *          235 - if any of the calibration data of type int16_t is equal to 0x7FFF
     *          236 - if any of the calibration data of type uint16_t is equal to 0xFFFF
     */
    esp_err_t get_cal_data();

    private:
    uint16_t get_ut();
    int32_t get_up();
    void delay();
};
#endif // BMP180_HPP