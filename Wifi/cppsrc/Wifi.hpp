#ifndef WIFI_HPP
#define WIFI_HPP

#include <iostream>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include <esp_wifi_types.h>
#include "nvs_flash.h"

#define GOT_IPV4_BIT        BIT(0)
#define GOT_IPV6_BIT        BIT(1)
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

typedef enum {
    WIFI_WPA_MODE_OFF=0,
    WIFI_WPA_MODE_2,
    WIFI_WPA_MODE_3
} wifi_wpa_mode_t;

class Wifi
{
    public:
    Wifi(std::string SSID, std::string pw, uint8_t retries = 0, wifi_wpa_mode_t wpa_mode=WIFI_WPA_MODE_2);
    esp_err_t initialize();
    esp_err_t disconnect();
    esp_err_t start();
    esp_err_t stop();
    static void eventHandler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
    static void on_got_ip(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);
    static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);                       
    ~Wifi();
    static char TAG[5];
    static uint8_t max_retries;
    static uint8_t num_retries;
    static esp_ip4_addr_t ip_addr;
    static EventGroupHandle_t wifiEventGroup;
    esp_netif_t *netif;

    private:
    bool init;
    wifi_mode_t mode;
    wifi_wpa_mode_t wpa_mode;
    std::string ssid;
    std::string password;
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
};

#endif // WIFI_HPP