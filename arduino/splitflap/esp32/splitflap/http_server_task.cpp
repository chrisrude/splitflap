#if HTTP_SERVER
#include "http_server_task.h"

#include "secrets.h"


HTTPServerTask::HTTPServerTask(
    SplitflapTask& splitflap_task,
    DisplayTask& display_task,
    Logger& logger,
    const uint8_t task_core) :
        Task("HTTP_Server", 16384, 1, task_core),
        splitflap_task_(splitflap_task),
        display_task_(display_task),
        logger_(logger),
        network_(logger) {
}

void HTTPServerTask::run() {
    logger_.log("Connecting to WiFi...");
    bool connected = network_.begin("splitflap", WIFI_SSID, WIFI_PASSWORD);
    if (connected) {
        logger_.log("Connected to WiFi");
    } else {
        logger_.log("Failed to connect to WiFi");
    }
    String last_status;

    while(1) {

        String wifi_status;
        wifi_status = "WiFi: ";
        if (!network_.is_wifi_connected()) {
            wifi_status += "Not connected";
        } else {
            wifi_status += "Connected to ";
            wifi_status += network_.get_ssid();
            wifi_status += " ";
            wifi_status += network_.get_ip_address();
        }
        if (last_status != wifi_status) {
            logger_.log(wifi_status.c_str());
            last_status = wifi_status;
        }

        if (!network_.is_wifi_connected()) {
            network_.reconnect_wifi();
        }

        delay(1000);
    }
}
#endif
