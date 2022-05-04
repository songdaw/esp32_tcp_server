/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WIFI_AP_H
#define __WIFI_AP_H

#ifdef __cplusplus
extern "C" {
#endif /*_cplusplus*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define ESP_WIFI_CHANNEL   1
#define MAX_STA_CONN       4

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void Esp_WifiApInit(void);

void Esp_WifiApStart(void);

void Esp_WifiApStop(void);


#ifdef __cplusplus
}
#endif /*_cplusplus*/

#endif /* __WIFI_AP_H */
