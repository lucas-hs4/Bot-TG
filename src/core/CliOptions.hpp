#pragma once

#include <optional>
#include <string>

namespace tgbot {

struct CliOptions {
    std::string configPath = "config/device.json";
    std::string routinePath = "config/routines/dailies.json";
    std::string deviceOverride;
    bool debug = false;
    bool showHelp = false;
};

std::optional<CliOptions> parseCliArgs(int argc, char* argv[]);
void printCliHelp();

}