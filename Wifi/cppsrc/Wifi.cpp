#include "Wifi.hpp"

char Wifi::TAG[] = "Wifi"; 
uint8_t Wifi::max_retries = 3;
uint8_t Wifi::num_retries = 0;
esp_ip4_addr_t Wifi::ip_addr;
EventGroupHandle_t Wifi::wifiEventGroup = NULL;

Wifi::Wifi(uint8_t retries, wifi_mode_t wifi_mode)
{
    netif       = nullptr; 
    mode        = wifi_mode;
    init        = false;
    max_retries = retries;
}

Wifi::~Wifi()
{
    esp_wifi_deinit();
    esp_netif_deinit();
}

esp_err_t Wifi::initialize(std::string SSID, std::string pw, wifi_wpa_mode_t wpa_mode)
{
    ssid            = SSID;
    password        = pw;
    this->wpa_mode  = wpa_mode;
    uint8_t retries = 0;

    while(esp_netif_init() != ESP_OK)
    {
        retries++;
        esp_netif_deinit();
        if(retries >= max_retries)
        {
            ESP_LOGE(TAG, "netif init");
            return ESP_FAIL;
        }
    }

    return create_default_event_loop();
}

esp_err_t Wifi::create_default_event_loop()
{
    if( esp_event_loop_create_default() != ESP_OK )
    {
        ESP_LOGE(TAG, "create event loop default STA");
        return ESP_FAIL;
    }
    
    init = true;
    return ESP_OK;
}

esp_err_t Wifi::connect()
{
    if(wifiEventGroup == NULL)
    {
        wifiEventGroup = xEventGroupCreate();
    }

    ESP_ERROR_CHECK( start() );

    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                            pdTRUE,
                                            pdFALSE,
                                            portMAX_DELAY);
                                           
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", ssid.c_str());
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "unable to connect to ap SSID:%s", ssid.c_str());
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;
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
    if(init)
    {
        ESP_LOGI(TAG, "Starting Wifi");      
        wifi_init_config_t wifi_init_config  = WIFI_INIT_CONFIG_DEFAULT();
        esp_wifi_init(&wifi_init_config);
        // ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

        if( mode == WIFI_MODE_STA )
        {
            return sta();
        } else if( mode == WIFI_MODE_AP ) 
        {
            return ap();
        }
        
    } else {
        ESP_LOGE(TAG, "start without init");
        return ESP_ERR_NOT_SUPPORTED;
    }

    return ESP_OK;
}

esp_err_t Wifi::ap()
{
    netif = esp_netif_create_default_wifi_ap();
    if( netif == nullptr ) 
    {
        ESP_LOGE(TAG, "create default AP");
        return ESP_FAIL;
    }
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &Wifi::eventHandlerAp,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config;
    const char* ss = ssid.c_str();
    memcpy(wifi_config.ap.ssid, ss, ssid.length());

    const char* pw = password.c_str();
    memcpy(wifi_config.ap.password, pw, password.length());

    wifi_config.ap.ssid_len = ssid.length();
    wifi_config.ap.channel  = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (password.length() == 0) {
        {wifi_config.ap.authmode = WIFI_AUTH_OPEN;};
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished.");

    return ESP_OK;
}

esp_err_t Wifi::sta()
{
    esp_err_t err;

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_WIFI_STA();
    netif = esp_netif_new(&netif_config);
    assert(netif);
    esp_netif_attach_wifi_station(netif);
    esp_wifi_set_default_wifi_sta_handlers();
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                                ESP_EVENT_ANY_ID,
                                                &Wifi::eventHandlerSta,
                                                NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                                WIFI_EVENT_STA_DISCONNECTED,
                                                &Wifi::on_wifi_disconnect,
                                                this));

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                                IP_EVENT_STA_GOT_IP,
                                                &Wifi::on_got_ip,
                                                NULL));

    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    
    wifi_config_t wifi_config = {
        .sta = {
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    const char* tmp_ssid = ssid.c_str();
    memcpy(wifi_config.sta.ssid, tmp_ssid, strlen(tmp_ssid));
    const char* tmp_pw = password.c_str();
    memcpy(wifi_config.sta.password, tmp_pw, strlen(tmp_pw));

    switch( wpa_mode )
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
    if(err != ESP_OK)
    {
        return err;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    ESP_ERROR_CHECK(err);
    if(err != ESP_OK)
    {
        return err;
    }
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
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
    if( mode == WIFI_MODE_STA)
    {
        esp_event_handler_unregister(WIFI_EVENT, 
                                ESP_EVENT_ANY_ID, 
                                &Wifi::eventHandlerSta);
    }

    if( mode == WIFI_MODE_AP )
    {
        esp_event_handler_unregister(WIFI_EVENT, 
                                ESP_EVENT_ANY_ID, 
                                &Wifi::eventHandlerAp);
    }

    vEventGroupDelete(wifiEventGroup);
    wifiEventGroup = NULL;
    return esp_wifi_stop();
}

void Wifi::eventHandlerSta(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (num_retries < max_retries) {
            esp_wifi_connect();
            num_retries++;
            ESP_LOGI("Wifi", "retry to connect to the AP");
        } else {
            xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
        }
        ESP_LOGI("Wifi","connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("Wifi", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        num_retries = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

void Wifi::eventHandlerAp(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI("Wifi", "station leave");
        ESP_LOGI(TAG, "AID=%d", event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI("Wifi", "station leave");
        ESP_LOGI(TAG, "AID=%d", event->aid);
    }   
}

void Wifi::on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Got IP event!");
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    memcpy(&Wifi::ip_addr, &event->ip_info.ip, sizeof(Wifi::ip_addr));
    ESP_LOGI("Wifi", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
}

void Wifi::on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    Wifi *ptrWifi = (Wifi*) arg;
    esp_err_t err = ptrWifi->connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
}