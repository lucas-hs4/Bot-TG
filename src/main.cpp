#include <string>

#include "core/CliOptions.hpp"
#include "core/ConfigLoader.hpp"
#include "core/Logger.hpp"
#include "fsm/RoutineDef.hpp"

int main(int argc, char* argv[]) {
    auto& log = tgbot::Logger::instance();

    const auto cliOpt = tgbot::parseCliArgs(argc, argv);
    if (!cliOpt.has_value()) {
        tgbot::printCliHelp();
        return 1;
    }
    if (cliOpt->showHelp) {
        tgbot::printCliHelp();
        return 0;
    }

    if (cliOpt->debug) {
        log.setLevel(tgbot::LogLevel::Debug);
    }

    log.info("=== tgbot - Teste CLI + RoutineDef (Fase 2, Passo2.1) ===");

    auto cfgOpt = tgbot::ConfigLoader::loadDeviceConfig(cliOpt->configPath);
    if (!cfgOpt.has_value()) {
        return 1;
    }
    if (!cliOpt->deviceOverride.empty()) {
        cfgOpt->deviceId = cliOpt->deviceOverride;
    }

    const auto routineOpt = tgbot::loadRoutineFromFile(cliOpt->routinePath);
    if (!routineOpt.has_value()) {
        return 1;
    }

    log.info("Dispositivo alvo: " + cfgOpt->deviceId);
    log.info("Rotina carregada: " + routineOpt->name +
             " | Global handlers: " + std::to_string(routineOpt->globalHandlers.size()) +
             " | Estados: " + std::to_string(routineOpt->states.size()));

    return 0;
}