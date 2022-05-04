idf_component_register(SRCS "esp_tcp.c"
                            "tcp_task.c"
                            "libs/wifi/wifi_ap.c"
                            "libs/wifi/tcp_server.c"
                    INCLUDE_DIRS "."
                                 "libs/wifi")
