/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TCP_TASK_H
#define __TCP_TASK_H

#ifdef __cplusplus
extern "C" {
#endif /*_cplusplus*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* Private includes ----------------------------------------------------------*/
#include "tcp_server.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    volatile uint8_t writeReady;
    volatile uint8_t connect;
    int32_t clientSocket;
    uint32_t ip;
} TCP_TaskDef;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

int32_t Esp_TCPServerStart(void);


#ifdef __cplusplus
}
#endif /*_cplusplus*/

#endif /* __TCP_TASK_H */
