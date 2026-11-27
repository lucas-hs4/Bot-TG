#include "core/ConfigLoader.hpp"

#include <fstream>

#include "core/Logger.hpp"

namespace tgbot {

std::optional<nlohmann::json> ConfigLoader::loadJsonFile(const std::filesystem::path&path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::instance().error("Nao foi possivel abrir o arquivo JSON: " + path.string());
        return std::nullopt;
    }

    try {
        nlohmann::json data;
        file >> data;
        return data;
    } catch (const nlohmann::json::exception& e) {
        Logger::instance().error("Erro de parse no JSON (" + path.string() + "): " + e.what());
        return std::nullopt;
    }
}

std::optional<DeviceConfig> ConfigLoader::loadDeviceConfig(const std::filesystem::path&path) {
    const auto jsonOpt = loadJsonFile(path);
    if (!jsonOpt.has_value()) {
        return std::nullopt;
    }

    const auto& j = *jsonOpt;
    try {
        DeviceConfig cfg;
        cfg.adbPath = j.at("adb_path").get<std::string>();
        cfg.deviceId = j.value("device_id", "");
        cfg.expectedWidth = j.value("expected_width", 1280);
        cfg.expectedHeight = j.value("expected_height", 720);
        cfg.templatesDir = j.value("templates_dir", "assets/templates");
        return cfg;
    } catch (const nlohmann::json::exception& e) {
        Logger::instance().error("Campos invalidos em device.json: " + std::string(e.what()));
        return std::nullopt;
    }
}

}