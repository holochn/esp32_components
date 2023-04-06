#ifndef WATERLEVELSENSOR_HPP
#define WATERLEVELSENSOR_HPP

#include "FrequencySensor.hpp"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"

class WaterLevelSensor : public FrequencySensor
{
public:
    /**
     * @brief: Constructor of class WaterLevelSensor
     * @param: 
     *          config pointer to a pcnt_config_t configuration
     *          min_val start value for minimum value. Will be updated during calibration process
     *          max_val start value for maximum value. Will be updated during calibration process 
     *          sample_time Sample time in ms.
     */
    WaterLevelSensor(pcnt_config_t *config, 
                    uint32_t min_val = 0xFFFFFFFF, 
                    uint32_t max_val = 0,
                    uint64_t sample_time = 100);
    
    /**
     * @brief: get water level reading
     * @return: the water level reading as int8_t (0 to 100%)
     */
    int8_t getWaterLevel();

    /**
     * @brief: Function can be used to activate / deactivate storage of 
     *          learned values to NVS. NVS must be prepared by user
     * @param: useNVS   bool to activate/deactivate NVS usage. Defalt is off
     */
    void useNVS(bool bUseNVS);

private:
    bool storeValsToNVS;
};

#endif // WATERLEVELSENSOR_HPP