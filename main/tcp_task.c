/* Includes ------------------------------------------------------------------*/
#include "tcp_task.h"
#include "wifi_ap.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define TCP_TAG "TCP"

#define TCP_DBG                 1

#define Send_CMD                0x5AU
#define CLIENT_ON               0x01U
#define CLIENT_OFF              0x00U
#define SendLen                 0x02U

/* Private variables ---------------------------------------------------------*/
TaskHandle_t TCPTaskHandle;
TaskHandle_t TCPSendHandle;

#define MAX_CLIENT      MAX_STA_CONN
TCP_TaskDef ClientInfo[MAX_CLIENT];
int32_t ClientCnt = 0;

uint8_t SendBuff[SendLen];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int32_t Esp_TCPInit(void)
{
    int32_t ret = 0;
    memset(&ClientInfo, 0, sizeof(TCP_TaskDef)*MAX_CLIENT);

    return ret;
}


int32_t Esp_PutClient(int32_t sock, uint32_t addr)
{
    for (size_t i = 0; i < MAX_CLIENT; i++)
    {
        if (ClientInfo[i].connect == 0)
        {
            ClientInfo[i].ip = addr;
            ClientInfo[i].clientSocket = sock;
            ClientInfo[i].connect = 1;

            if (ClientCnt < MAX_CLIENT)
            {
                ClientCnt++;
            }

            if (ClientCnt == 1)
            {
                SendBuff[0] = Send_CMD;
                SendBuff[1] = CLIENT_ON;
            }

            xTaskNotifyGive(TCPSendHandle);
            ESP_LOGI(TCP_TAG, "Add client:%d %d", sock, addr);
            return 0;
        }
    }

    return -1;
}


int32_t Esp_PopClientIP(uint32_t addr)
{
    int32_t sock;

    for (size_t i = 0; i < MAX_CLIENT; i++)
    {
        if (ClientInfo[i].ip == addr)
        {
            sock = ClientInfo[i].clientSocket;
            ClientInfo[i].ip = 0;
            ClientInfo[i].clientSocket = -1;

            if (ClientInfo[i].connect == 1)
            {
                ClientInfo[i].connect = 0;

                if (ClientCnt > 0)
                {
                    ClientCnt--;
                }

                shutdown(sock, 0);
                close(sock);
                ESP_LOGI(TCP_TAG, "Pop client:%d", sock);

                if (ClientCnt == 0)
                {
                    SendBuff[0] = Send_CMD;
                    SendBuff[1] = CLIENT_OFF;
                }
            }

            return 0;
        }
    }

    return -1;
}


int32_t Esp_PopClient(int32_t sock)
{
    for (size_t i = 0; i < MAX_CLIENT; i++)
    {
        if (ClientInfo[i].clientSocket == sock)
        {
            ClientInfo[i].clientSocket = -1;
            ClientInfo[i].ip = 0;

            if (ClientInfo[i].connect == 1)
            {
                ClientInfo[i].connect = 0;

                if (ClientCnt > 0)
                {
                    ClientCnt--;
                }

                shutdown(sock, 0);
                close(sock);
                ESP_LOGI(TCP_TAG, "Pop client:%d", sock);

                if (ClientCnt == 0)
                {
                    SendBuff[0] = Send_CMD;
                    SendBuff[1] = CLIENT_OFF;
                }
            }

            return 0;
        }
    }

    return -1;
}


void Esp_TCPRecvTask(void * pvParameters)
{
    int sock = (int)pvParameters;
    char rx_buffer[128];
    int len;

    ESP_LOGI(TCP_TAG, "Recv from:%d", sock);

    for(;;)
    {
        len = recv(sock, rx_buffer, sizeof(rx_buffer), 0);
        if (len < 0) {
            ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
            Esp_PopClient(sock);
            break;
        } else if (len == 0) {
            ESP_LOGW(TCP_TAG, "Connection closed");
            Esp_PopClient(sock);
            break;
        } else {
        #if TCP_DBG
            ESP_LOGI(TCP_TAG, "Recv:%d", len);
            esp_log_buffer_hex(TCP_TAG, rx_buffer, len);
        #endif
        }
    }

    vTaskDelete(NULL);
}


void Esp_TCPSendTask(void * pvParameters)
{
    TickType_t xTicksToWait = portMAX_DELAY;

    for(;;)
    {
        if (ClientCnt > 0)
        {
            ulTaskNotifyTake(pdTRUE, xTicksToWait);
        } else {
        #if TCP_DBG
            ESP_LOGI(TCP_TAG, "Send wait connect");
        #endif
            ulTaskNotifyTake(pdTRUE, xTicksToWait);
        }
    }
}


void Esp_TCPServerTask(void * pvParameters)
{
    int32_t serverSocket;
    int32_t clientSocket;
    uint32_t ip_addr;

    ESP_ERROR_CHECK(Esp_TCPServerInit(&serverSocket));
    for(;;)
    {
        clientSocket = Esp_TCPServerAccept(serverSocket, &ip_addr);
        if (clientSocket < 0)
        {
            ESP_LOGE(TCP_TAG, "Unable to accept");
        } else {
            if (Esp_PutClient(clientSocket, ip_addr) == 0)
            {
                int32_t ret;

                ret = xTaskCreate(Esp_TCPRecvTask, 
                                    "Recv",
                                    4096,
                                    (void*)clientSocket, 
                                    tskIDLE_PRIORITY + 1,
                                    NULL);
                if (ret != pdPASS)
                {
                    ESP_LOGE(TCP_TAG, "Task create error");
                }
            } else {
                ESP_LOGE(TCP_TAG, "Too many clients");
            }
        }
    }
}


int32_t Esp_TCPServerStart(void)
{
    int32_t ret;

    ESP_LOGI(TCP_TAG, "Esp_TCPTaskStart");
    ret = Esp_TCPInit();
    if (ret)
    {
        ESP_LOGE(TCP_TAG, "Init error");
        return ret;
    }

    ret = xTaskCreate(Esp_TCPServerTask, 
                      TCP_TAG, 
                      4096,
                      NULL, 
                      tskIDLE_PRIORITY + 1,
                      &TCPTaskHandle);
    if (ret != pdPASS)
    {
        ESP_LOGE(TCP_TAG, "Task create error");
        return ret;
    }

    ESP_LOGI(TCP_TAG, "Create RW task");

    ret = xTaskCreate(Esp_TCPSendTask, 
                      "Send", 
                      4096,
                      NULL, 
                      tskIDLE_PRIORITY + 1,
                      &TCPSendHandle);
    if (ret != pdPASS)
    {
        ESP_LOGE(TCP_TAG, "Task create error");
        return ret;
    }

    return 0;
}
