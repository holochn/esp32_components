#include "SntpClient.hpp"

char SntpClient::TAG[] = "SntpClient";

SntpClient::SntpClient()
{
    
}

void SntpClient::sntp_cb(void* arg)
{
    while(true)
    {
        if( sntp(arg) == ESP_FAIL )
        {
            vTaskDelay(  30000 / portTICK_PERIOD_MS);
        } else {
            continue;
            // or delay for 24 hours
            // vTaskDelay(  24 * 60 * 60 * 1000 / portTICK_PERIOD_MS);
        }
    }

    vTaskDelete( NULL );
}

esp_err_t SntpClient::sntp(void* arg)
{
    time_t now;
    time_t lastTimeSync;
    struct tm timeinfo;
    struct tm lastTimeSyncTimeInfo;
    char strftime_buf[64];
    struct timeval outdelta;

    time(&now);
    localtime_r(&now, &timeinfo);

    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    
    setenv("TZ", TIMEZONE, 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);

    // change system time
    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
            adjtime(NULL, &outdelta);
            ESP_LOGI(TAG, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
                        (long)outdelta.tv_sec,
                        outdelta.tv_usec/1000,
                        outdelta.tv_usec%1000);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
    // save the timestamp of last snyc
    time(&lastTimeSync);
    localtime_r(&lastTimeSync, &lastTimeSyncTimeInfo);

    if(timeinfo.tm_year > 70) {
        ESP_LOGI(TAG, "System time ok");
        return ESP_OK;
    } else {
        ESP_LOGI(TAG, "Restart time update process");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void SntpClient::time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Time synchronization event");
}

void SntpClient::obtain_time(void)
{
    time_t now = 0;
    struct tm timeinfo = { 0,0,0,0,0,0,0,0,0 };
    int retry = 0;
    const int retry_count = 10;

    initialize_sntp();

    // wait for time to be set
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Wait for sys time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

void SntpClient::initialize_sntp(void)
{
    if( sntp_enabled() ) {
		sntp_stop();
	}

	ESP_LOGI(TAG, "Init SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}