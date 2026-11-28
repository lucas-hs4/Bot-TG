#include <csignal>
#include <string>

#include "core/AdbController.hpp"
#include "core/CliOptions.hpp"
#include "core/ConfigLoader.hpp"
#include "core/Logger.hpp"
#include "core/Randomizer.hpp"
#include "fsm/RoutineDef.hpp"
#include "fsm/StateMachine.hpp"
#include"vision/ScreenDetector.hpp"

namespace {
tgbot::StateMachine* g_activeFsm = nullptr;

void handleSignal(int) {
    if (g_activeFsm != nullptr) {
        g_activeFsm->requestStop();
    }
}
}

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

    tgbot::ScreenDetector detector;
    detector.loadTemplatesFromDir(cfgOpt->templatesDir);

    tgbot::AdbController adb(cfgOpt->adbPath, cfgOpt->deviceId);
    if (!adb.checkConnection()) {
        log.error("Emulador nao conectado: " + cfgOpt->deviceId);
        return 1;
    }

    tgbot::Randomizer rng;
    tgbot::StateMachine fsm(adb, detector, rng);

    g_activeFsm = &fsm;
    std::signal(SIGINT, handleSignal);

    const bool ok = fsm.run(*routineOpt, 3);
    g_activeFsm = nullptr;

    return ok ? 0 : 1;

}