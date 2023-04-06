#ifndef SNTPCLIENT_HPP
#define SNTPCLIENT_HPP

#include "esp_log.h"
#include "esp_sntp.h"

// https://remotemonitoringsystems.ca/time-zone-abbreviations.php
#if CONFIG_LANGUAGE == GARDOMATIC_DE_DE
#define TIMEZONE "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"
#elif CONFIG_LANGUAGE == GARDOMATIC_EN_EN
#define TIMEZONE "GMT+0BST-1,M3.5.0/01:00:00,M10.5.0/02:00:00"
#endif

class SntpClient
{
    public:
    SntpClient();
    static void sntp_cb(void* arg);
    static esp_err_t sntp(void* arg);
    static void time_sync_notification_cb(struct timeval *tv);
    static void obtain_time(void);
    static void initialize_sntp(void);
    static char TAG[];

    private:
    
};

#endif // SNTPCLIENT_HPP