/* Includes ------------------------------------------------------------------*/
#include "wifi_ap.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    uint32_t ip;        //IPv4
    uint8_t mac[6];
    uint8_t connect;
} AP_TaskDef;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define WIFI_TAG    "WIFI"

#define ESP_WIFI_SSID      "ESP32"
#define ESP_WIFI_PASS      "12345678"

/* Private variables ---------------------------------------------------------*/
AP_TaskDef APInfo[MAX_STA_CONN];
int8_t APIndex = -1;

/* Private function prototypes -----------------------------------------------*/

int32_t Esp_PutAPClientIP(uint32_t addr) 
{
    if (APIndex < 0)
    {
        return -1;
    }

    APInfo[APIndex].ip = addr;
    APIndex = -1;
    ESP_LOGI(WIFI_TAG, "Put AP:%d", addr);

    return 0;
}


int32_t Esp_PutAPClinetMAC(uint8_t* mac)
{
    for (size_t i = 0; i < MAX_STA_CONN; i++)
    {
        if (APInfo[i].connect == 0)
        {
            memcpy(APInfo[i].mac , mac, 6);
            APInfo[i].connect = 1;
            APIndex = i;

            return 0;
        }
    }

    return -1;
}


__attribute__((__weak__)) int32_t Esp_PopClientIP(uint32_t addr)
{
    return 0;
}


int32_t Esp_PopAPClinetMAC(uint8_t* mac)
{
    for (size_t i = 0; i < MAX_STA_CONN; i++)
    {
        if (memcmp(mac, APInfo[i].mac, 6) == 0)
        {
            if (APInfo[i].connect == 1)
            {
                APInfo[i].connect = 0;

                Esp_PopClientIP(APInfo[i].ip);

                ESP_LOGI(WIFI_TAG, "Pop AP:%d", APInfo[i].ip);
                APInfo[i].ip = 0;
            }

            return 0;
        }
    }

    return -1;
}


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(WIFI_TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
            Esp_PutAPClinetMAC(event->mac);
        } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
            wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
            ESP_LOGI(WIFI_TAG, "station "MACSTR" leave, AID=%d",
                    MAC2STR(event->mac), event->aid);
            Esp_PopAPClinetMAC(event->mac);
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_AP_STAIPASSIGNED) {
            ip_event_ap_staipassigned_t* event = (ip_event_ap_staipassigned_t*) event_data;
            ESP_LOGI(WIFI_TAG, "IP assigned");
            // Add to table
            Esp_PutAPClientIP(event->ip.addr);
        }
    }
}


void Esp_WifiApInit(void)
{
    memset(&APInfo , 0, sizeof(AP_TaskDef) * MAX_STA_CONN);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_AP_STAIPASSIGNED,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t wifi_config = {
        .ap = {
            .channel = ESP_WIFI_CHANNEL,
            .password = ESP_WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    if (strlen(ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN; 
    }

    uint8_t ApMac[6];

    esp_wifi_get_mac(ESP_IF_WIFI_AP, ApMac);
    sprintf((char *)wifi_config.ap.ssid , "%s_%02X%02X" , ESP_WIFI_SSID , 
                                           ApMac[4] , ApMac[5]);
    wifi_config.ap.ssid_len = strlen(ESP_WIFI_SSID) + 5;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
}


void Esp_WifiApStart(void)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(WIFI_TAG, "Esp_WifiApStart");
}


void Esp_WifiApStop(void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(WIFI_TAG, "Esp_WifiApStop");
}


