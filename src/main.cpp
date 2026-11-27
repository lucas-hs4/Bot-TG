#include <chrono>
#include <string>

#include "core/AdbController.hpp"
#include "core/ConfigLoader.hpp"
#include "core/Logger.hpp"
#include "vision/ScreenDetector.hpp"

int main() {
    auto& log = tgbot::Logger::instance();
    log.setLevel(tgbot::LogLevel::Debug);
    log.info("=== tgbot - Teste ConfigLoader + ScreenDetector (Fase 1, Passo 1.4) ===");

    const auto cfgOpt = tgbot::ConfigLoader::loadDeviceConfig("config/device.json");
    if (!cfgOpt.has_value()) {
        log.error("Falha ao carregar config/device.json.");
        return 1;
    }
    const auto& cfg = *cfgOpt;

    tgbot::ScreenDetector detector;
    detector.loadTemplatesFromDir(cfg.templatesDir);

    tgbot::AdbController adb(cfg.adbPath, cfg.deviceId);
    if (!adb.checkConnection()) {
        log.error("Emulador nao conectado: " + cfg.deviceId);
        return 1;
    }

    const cv::Mat frame = adb.captureScreen();
    if (frame.empty()) {
        log.error("Falha ao capturar tela.");
        return 1;
    }

    log.info("Captura OK: " + std::to_string(frame.cols) + "x" + std::to_string(frame.rows) +
             " | Templates em cache: " + std::to_string(detector.templateCount()));

    // Se voce ja tiver colocado algum PNG em assets/templates/teste.png, pode testar aqui:
    if (detector.hasTemplate("teste.png")) {
        const auto t0 = std::chrono::steady_clock::now();
        const auto res = detector.findTemplate(frame, "teste.png", 0.85);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();

        log.info("Busca 'teste.png': found=" + std::string(res.found ? "true" : "false") +
                 " | conf=" + std::to_string(res.confidence) +
                 " | tempo=" + std::to_string(ms) + " ms");
    }

    return 0;
}