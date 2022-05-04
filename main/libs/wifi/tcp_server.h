/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif /*_cplusplus*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

int32_t Esp_TCPServerInit(int32_t* lsocket);

int32_t Esp_TCPServerAccept(int32_t lsocket, uint32_t* addr);

int32_t Esp_TCPServerSend(int32_t sock, uint8_t* data, int32_t len);


#ifdef __cplusplus
}
#endif /*_cplusplus*/

#endif /* __TCP_SERVER_H */
