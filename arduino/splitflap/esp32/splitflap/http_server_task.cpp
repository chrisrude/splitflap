#if HTTP_SERVER
#include "http_server_task.h"

#define PARAM_MESSAGE "message"

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
        network_(logger),
        server_(80) {
}

void HTTPServerTask::run() {
    logger_.log("Connecting to WiFi...");
    bool connected = network_.begin("splitflap", WIFI_SSID, WIFI_PASSWORD);
    if (connected) {
        logger_.log("Connected to WiFi");
    } else {
        logger_.log("Failed to connect to WiFi");
    }


    bool spiffs_ok = SPIFFS.begin();
    if (!spiffs_ok) {
        logger_.log("SPIFFS Mount Failed");
        return;
    }

    server_.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html");
    // todo: precompress files, add gzip header
    // AsyncStaticWebHandler* static_handler =  ....
    // static_handler.addHeader("Content-Encoding", "gzip");

    // Send a GET request to <IP>/get?message=<message>
    server_.on("/text", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message;
        // todo: share some code with display_task
        request->send(200, "text/plain", "THE_TEXT");
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server_.on("/set", HTTP_POST, [this](AsyncWebServerRequest *request){
        String message;
        if (!request->hasParam(PARAM_MESSAGE, true)) {
            request->send(400, "text/plain", "ERROR: No message sent");
            return;
        }
        message = request->getParam(PARAM_MESSAGE, true)->value();

        // make sure message is exactly NUM_MODULES long, truncating
        // it if it's too long, or padding it with spaces if it's too short
        if (message.length() > NUM_MODULES) {
            message = message.substring(0, NUM_MODULES);
        } else if (message.length() < NUM_MODULES) {
            message += String(NUM_MODULES - message.length(), ' ');
        }

        // lower case all letters
        message.toLowerCase();

        // if any letter is not in flaps[], replace it with a space
        for (int i = 0; i < message.length(); i++) {
            bool found = false;
            for (int j = 0; j < NUM_FLAPS; j++) {
                if (message[i] == flaps[j]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                message[i] = ' ';
            }
        }

        logger_.log("Setting text to: ");
        logger_.log(message.c_str());

        // there are two rows, figure out how many modules
        // are in the first row
        const int FIRST_ROW_MODULES = NUM_MODULES / 2;

        // reverse all of the letters in the first row
        for (int i = 0; i < FIRST_ROW_MODULES / 2; i++) {
            char temp = message[i];
            message[i] = message[FIRST_ROW_MODULES - i - 1];
            message[FIRST_ROW_MODULES - i - 1] = temp;
        }

        splitflap_task_.showString(message.c_str(), message.length());
        request->send(200, "text/plain", "OK");
    });

    server_.on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Resetting");
        logger_.log("Resetting");
        splitflap_task_.resetAll();
    });

    server_.onNotFound( [](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });

    server_.begin();

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
            wifi_status += WiFi.localIP().toString();
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
