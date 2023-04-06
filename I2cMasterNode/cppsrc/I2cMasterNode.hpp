#ifndef I2CMASTERNODE_HPP
#define I2CMASTERNODE_HPP

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#define I2C_ACK_CHECK_EN    true
#define I2C_ACK_CHECK_DIS   false

/**
 * @brief: Base class, which acts as a I2C node commanded from a master
 * 
 */
class I2cMasterNode {
    public:
    /**
     * @brief: Constructor a class I2cMaster node
     * @param:  
     *          sda The GPIO of sda pin
     *          scl The GPIO of scl pin
     *          freq The I2C frequency
     *          i2c_port the I2C port. Default value is 0
     *          pullup_enable Flag to enable/disable pul up ESP's resistors for sda and scl. Default value is true
     */
    I2cMasterNode(gpio_num_t sda, gpio_num_t scl, uint32_t freq, uint8_t i2c_port=0, bool pullup_enable=true);
    
    /**
     * @brief:  Reads a register of a I2C node
     * @param:  
     *          addr    The I2C address of remote node
     *          reg     The register address to be read
     * @return: uint8_t register value
     */
    uint8_t i2c_register_read8(uint8_t addr, uint8_t reg);
    
    /**
     * @brief:  Reads a register of a I2C node.
     * @param:  addr    The I2C address of remote node
     *          reg     The register address to be read
     * @return: uint16_t register value
     */
    uint16_t i2c_register_read16(uint8_t addr, uint8_t reg);

    /**
     * @brief:  Reads a register of a I2C node wit little endian encoding.
     * @param:  addr    The I2C address of remote node
     *          reg     The register address to be read
     * @return: uint16_t register value 
     */
    uint16_t i2c_register_read16_lien(uint8_t addr, uint8_t reg);

    /**
     * @brief:  Writes a register of a I2C node and returns a uint8_t value
     * @param:  addr    The I2C address of remote node
     *          reg     The register address to be read
     * @return: ESP_OK: Success
     *          ESP_ERR_INVALID_ARG Parameter error
     *          ESP_FAIL Driver install error
     *          ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
     *          ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
     */
    esp_err_t i2c_register_write(uint8_t addr, uint8_t reg, uint8_t reg_val);

    private:
    esp_err_t i2c_master_initialize();

    i2c_config_t i2c_config;
    gpio_num_t i2c_sda;
    gpio_num_t i2c_scl;
    uint32_t i2c_freq;
    bool i2c_pullup_enable;
    i2c_port_t   port;
};

#endif // I2CMASTERNODE_HPP