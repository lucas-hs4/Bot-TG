#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace tgbot {

struct DeviceConfig {
    std::string adbPath;
    std::string deviceId;
    int expectedWidth = 1280;
    int expectedHeight = 720;
    std::string templatesDir = "assets/templates";
};

class ConfigLoader {
public:
    static std::optional<nlohmann::json> loadJsonFile(const std::filesystem::path&path);
    static std::optional<DeviceConfig> loadDeviceConfig(const std::filesystem::path&path);
};

}