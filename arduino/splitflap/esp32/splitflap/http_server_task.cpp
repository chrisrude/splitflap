#if HTTP_SERVER
#include "http_server_task.h"

#define PARAM_MESSAGE "message"

#include <json11.hpp>

#include "secrets.h"

#include "src/splitflap_module_data.h"

using namespace json11;

class FlapStatus {
public:
    State state;
    uint8_t flap_index;
    bool moving;
    bool home_state;
    uint8_t count_unexpected_home;
    uint8_t count_missed_home;

    FlapStatus(const SplitflapModuleState& module_state) :
        state(module_state.state),
        flap_index(module_state.flap_index),
        moving(module_state.moving),
        home_state(module_state.home_state),
        count_unexpected_home(module_state.count_unexpected_home),
        count_missed_home(module_state.count_missed_home) {
    }

    std::string state_string() const {
        switch (state) {
            case NORMAL:
                return "normal";
            case LOOK_FOR_HOME:
                return "look_for_home";
            case SENSOR_ERROR:
                return "sensor_error";
            case PANIC:
                return "panic";
            case STATE_DISABLED:
                return "state_disabled";
            default:
                return "unknown";
        }
    }

    Json to_json() const {
        return Json::object {
            { "state", Json(state_string())},
            { "flap_index", flap_index },
            { "moving", moving },
            { "home_state", home_state },
            { "count_unexpected_home", count_unexpected_home },
            { "count_missed_home", count_missed_home },
        };
    }
};

HTTPServerTask::HTTPServerTask(
    SplitflapTask& splitflap_task,
    DisplayTask& display_task,
    Logger& logger,
    const uint8_t task_core) :
        Task("HTTP_Server", 32768, 1, task_core),
        splitflap_task_(splitflap_task),
        display_task_(display_task),
        logger_(logger),
        network_(logger),
        server_(80) {
}

void sanitize_string(String& msg) {
    // make sure message is exactly NUM_MODULES long, truncating
    // it if it's too long, or padding it with spaces if it's too short
    if (msg.length() > NUM_MODULES) {
        msg = msg.substring(0, NUM_MODULES);
    } else if (msg.length() < NUM_MODULES) {
        for (int i = msg.length(); i < NUM_MODULES; i++) {
            msg += " ";
        }
    }

    // also, make it all lower-case
    msg.toLowerCase();

    // if any letter is not in flaps[], replace it with a space
    for (int i = 0; i < msg.length(); i++) {
        bool found = false;
        for (int j = 0; j < NUM_FLAPS; j++) {
            if (msg[i] == flaps[j]) {
                found = true;
                break;
            }
        }
        if (!found) {
            msg[i] = ' ';
        }
    }
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
    // todo: pre-compress files, add gzip header
    // AsyncStaticWebHandler* static_handler =  ....
    // static_handler.addHeader("Content-Encoding", "gzip");

    // Send a GET request to <IP>/get?message=<message>
    server_.on("/text", HTTP_GET, [this] (AsyncWebServerRequest *request) {
        // todo: share some code with display_task
        request->send(200, "text/plain", last_text_);
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server_.on("/set", HTTP_POST, [this](AsyncWebServerRequest *request){
        String message;
        if (!request->hasParam(PARAM_MESSAGE, true)) {
            request->send(400, "text/plain", "ERROR: No message sent");
            return;
        }
        message = request->getParam(PARAM_MESSAGE, true)->value();

        sanitize_string(message);

        logger_.log("Setting text to: ");
        logger_.log(message.c_str());

        String unreversed_message = message;

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

        last_text_ = unreversed_message;

        request->send(200, "text/plain", unreversed_message);
    });

    server_.on("/flaps", HTTP_GET, [this](AsyncWebServerRequest *request){
        String message;
        for (int i = 0; i < NUM_FLAPS; i++) {
            message += (char)flaps[i];
        }
        request->send(200, "text/plain", message);
    });

    server_.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        // send a JSON array of flap statuses
        Json::array statuses;
        SplitflapState state = splitflap_task_.getState();
        for (int i = 0; i < NUM_MODULES; i++) {
            statuses.push_back(FlapStatus(state.modules[i]).to_json());
        }
        std::string json_str = Json(statuses).dump();

        request->send(200, "application/json", json_str.c_str());
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

    print_status();

    while(1) {

        if (!network_.is_wifi_connected()) {
            network_.reconnect_wifi();
            print_status();
        }
        delay(10000);
    }
}


void HTTPServerTask::print_status() {
    static boolean last_connected = false;

    boolean connected = network_.is_wifi_connected();
    if (connected == last_connected) {
        return;
    }

    String wifi_status;
    if (connected) {
        wifi_status = "WiFi: connected to ";
        wifi_status += network_.get_ssid();
        wifi_status += " ";
        wifi_status += WiFi.localIP().toString();
    } else {
        wifi_status = "WiFi: reconnecting to ";
        wifi_status += network_.get_ssid();
        wifi_status += "...";
    }
    logger_.log(wifi_status.c_str());
    last_connected = connected;
}
#endif
