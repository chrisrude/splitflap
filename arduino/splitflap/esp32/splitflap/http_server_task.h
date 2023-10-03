#pragma once

#include <Arduino.h>

#include "../core/logger.h"
#include "../core/splitflap_task.h"
#include "../core/task.h"

#include "display_task.h"
#include "network.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>


class HTTPServerTask : public Task<HTTPServerTask> {
    friend class Task<HTTPServerTask>; // Allow base Task to invoke protected run()

    public:
        HTTPServerTask(SplitflapTask& splitflap_task, DisplayTask& display_task, Logger& logger, const uint8_t task_core);

    protected:
        void run();
        void print_status();

    private:
        SplitflapTask& splitflap_task_;
        DisplayTask& display_task_;
        Logger& logger_;
        Network network_;
        AsyncWebServer server_;
};
