#include "NonVolatileStorage.hpp"

NonVolatileStorage::NonVolatileStorage(std::string t_storage_name) : 
    m_storage_name{t_storage_name}
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}

NonVolatileStorage::~NonVolatileStorage()
{
}

template <typename T>
esp_err_t NonVolatileStorage::read(std::string name, T &var)
{
    esp_err_t err;
    std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(m_storage_name.c_str(), NVS_READWRITE, &err);
    err = handle->get_item(name.c_str(), var);

    return err;
}

template <typename T>
esp_err_t NonVolatileStorage::write(std::string name, T val)
{
    esp_err_t err;
    std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(m_storage_name.c_str(), NVS_READWRITE, &err);
    
    err = handle->set_item(name.c_str(), val);
    if(err != ESP_OK)
    {
        return err;
    }
    err = handle->commit();
    return err;
}
