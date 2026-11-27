#include <chrono>
#include <string>

#include <opencv2/highgui.hpp>

#include "core/AdbController.hpp"
#include "core/Logger.hpp"

int main() {
    auto& log = tgbot::Logger::instance();
    log.setLevel(tgbot::LogLevel::Debug);
    log.info("=== tgbot - Teste ADB em RAM (Fase 1, Passo 1.3) ===");

    const std::string adbPath = R"(C:\LDPlayer\LDPlayer9\adb.exe)";
    const std::string deviceId = "emulator-5554";

    tgbot::AdbController adb(adbPath, deviceId);

    if (!adb.checkConnection()) {
        log.error("Emulador nao conectado! Verifique se o LDPlayer esta aberto com ADB ativado.");
        return 1;
    }
    log.info("Conexao ADB confirmada com: " + adb.deviceId());

    const auto start = std::chrono::steady_clock::now();
    const cv::Mat frame = adb.captureScreen();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    if (frame.empty()) {
        log.error("Falha ao capturar tela do emulador.");
        return 1;
    }

    log.info("Screenshot em RAM OK! Dimensoes: " +
             std::to_string(frame.cols) + "x" + std::to_string(frame.rows) +
             " | Tempo: " + std::to_string(elapsedMs) + " ms");

    if (frame.cols != 1280 || frame.rows != 720) {
        log.warn("Resolucao diferente do padrao 1280x720! Ajuste o emulador.");
    }

    cv::imshow("tgbot - Captura em RAM (pressione qualquer tecla para fechar)", frame);
    cv::waitKey(3000);
    return 0;
}