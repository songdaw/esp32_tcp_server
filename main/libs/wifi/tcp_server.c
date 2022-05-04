/* Includes ------------------------------------------------------------------*/
#include "tcp_server.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define SER_TAG     "SERVER"

#define TCP_PORT    8266

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

int32_t Esp_TCPServerInit(int32_t* lsocket)
{
    struct sockaddr_in dest_addr;

    int32_t listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        ESP_LOGE(SER_TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    ESP_LOGI(SER_TAG, "Socket created");

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(TCP_PORT);

    int32_t err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(SER_TAG, "Socket unable to bind: errno %d", errno);
        close(listen_sock);
        return -1;
    }
    ESP_LOGI(SER_TAG, "Socket bound, port %d", TCP_PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(SER_TAG, "Error occurred during listen: errno %d", errno);
        close(listen_sock);
        return -1;
    }

    *lsocket = listen_sock;

    return 0;
}


int32_t Esp_TCPServerAccept(int32_t lsocket, uint32_t* addr)
{
    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t addr_len = sizeof(source_addr);

    int32_t sock = accept(lsocket, (struct sockaddr *)&source_addr, &addr_len);
    if (sock < 0) {
        ESP_LOGE(SER_TAG, "Unable to accept connection: errno %d", errno);
    }

    *addr = ((struct sockaddr_in *)&source_addr)->sin_addr.s_addr;

    return sock;
}


int32_t Esp_TCPServerSend(int32_t sock, uint8_t* data, int32_t len)
{
    int to_write = len;

    while (to_write > 0) {
        int written = send(sock, data + (len - to_write), to_write, 0);
        if (written < 0) {
            ESP_LOGE(SER_TAG, "Error occurred during sending: errno %d", errno);
            return -1;
        }
        to_write -= written;
    }

    return 0;
}

