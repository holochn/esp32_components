#include "Wifi.hpp"

char Wifi::TAG[] = "Wifi";
uint8_t Wifi::max_retries = 3;
uint8_t Wifi::num_retries = 0;
esp_ip4_addr_t Wifi::ip_addr;
EventGroupHandle_t Wifi::wifiEventGroup = NULL;

Wifi::Wifi(std::string SSID, std::string pw, uint8_t retries, wifi_wpa_mode_t wpa_mode)
{
    netif = nullptr;
    init = false;
    max_retries = retries;
    ssid = SSID;
    password = pw;
    this->wpa_mode = wpa_mode;
}

Wifi::~Wifi()
{
    esp_wifi_deinit();
    esp_netif_deinit();
}

esp_err_t Wifi::initialize()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    while (esp_netif_init() != ESP_OK)
    {
        num_retries++;
        esp_netif_deinit();
        if (num_retries >= max_retries)
        {
            ESP_LOGE(TAG, "netif init");
            return ESP_FAIL;
        }
    }

    if (esp_event_loop_create_default() != ESP_OK)
    {
        ESP_LOGE(TAG, "create event loop default STA");
        return ESP_FAIL;
    }

    if (wifiEventGroup == NULL)
    {
        wifiEventGroup = xEventGroupCreate();
    }

    esp_err_t err;

    netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    if( esp_wifi_init(&wifi_init_config) != ESP_OK) {
        ESP_LOGE(TAG, "Wifi not init");
        return ESP_ERR_WIFI_NOT_INIT;
    }

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               &Wifi::eventHandler,
                                               NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               WIFI_EVENT_STA_DISCONNECTED,
                                               &Wifi::on_wifi_disconnect,
                                               this));

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               &Wifi::on_got_ip,
                                               NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    const char *tmp_ssid = ssid.c_str();
    memcpy(wifi_config.sta.ssid, tmp_ssid, strlen(tmp_ssid));
    const char *tmp_pw = password.c_str();
    memcpy(wifi_config.sta.password, tmp_pw, strlen(tmp_pw));

    switch (wpa_mode)
    {
    case WIFI_WPA_MODE_OFF:
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_PSK;
        break;
    case WIFI_WPA_MODE_2:
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        break;
    case WIFI_WPA_MODE_3:
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA3_PSK;
        break;
    default:
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        break;
    }

    ESP_LOGI(TAG, "Connecting to %s", wifi_config.sta.ssid);
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    ESP_ERROR_CHECK(err);
    if (err != ESP_OK)
    {
        return err;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    ESP_ERROR_CHECK(err);
    if (err != ESP_OK)
    {
        return err;
    }

    return ESP_OK;
}

esp_err_t Wifi::disconnect()
{
    stop();
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(netif));
    esp_netif_destroy(netif);
    netif = NULL;
    return ESP_OK;
}

esp_err_t Wifi::start()
{
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdTRUE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s", ssid.c_str());
        return ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "unable to connect to ap SSID:%s", ssid.c_str());
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t Wifi::stop()
{
    esp_event_handler_unregister(IP_EVENT,
                                 IP_EVENT_STA_GOT_IP,
                                 &Wifi::on_got_ip);
    esp_event_handler_unregister(WIFI_EVENT,
                                 WIFI_EVENT_STA_DISCONNECTED,
                                 &Wifi::on_wifi_disconnect);
    esp_event_handler_unregister(WIFI_EVENT, 
                                ESP_EVENT_ANY_ID, 
                                &Wifi::eventHandler);

    vEventGroupDelete(wifiEventGroup);
    wifiEventGroup = NULL;
    return esp_wifi_stop();
}

void Wifi::eventHandler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (num_retries < max_retries)
        {
            esp_wifi_connect();
            num_retries++;
            ESP_LOGI("Wifi", "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
        }
        ESP_LOGI("Wifi", "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI("Wifi", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        num_retries = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

void Wifi::on_got_ip(void *arg, esp_event_base_t event_base,
                     int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Got IP event!");
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    memcpy(&Wifi::ip_addr, &event->ip_info.ip, sizeof(Wifi::ip_addr));
    ESP_LOGI("Wifi", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
}

void Wifi::on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    Wifi *ptrWifi = (Wifi *)arg;
    esp_err_t err = ptrWifi->start();
    if (err == ESP_ERR_WIFI_NOT_STARTED)
    {
        return;
    }
}