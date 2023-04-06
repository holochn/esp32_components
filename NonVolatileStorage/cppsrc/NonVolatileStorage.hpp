#ifndef NVS_HPP
#define NVS_HPP

#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"

class NonVolatileStorage
{
private:
    std::string m_storage_name;
public:
    NonVolatileStorage(std::string t_storage_name);
    ~NonVolatileStorage();
    
    template <typename T>
    esp_err_t read(std::string name, T &var);
    
    template <typename T>
    esp_err_t write(std::string name, T val);
};


#endif // NVS_HPP