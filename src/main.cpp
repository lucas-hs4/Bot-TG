#include <opencv2/core/types.hpp>

#include "core/Logger.hpp"
#include "core/Randomizer.hpp"

int main() {
    auto& log = tgbot::Logger::instance();
    log.setLevel(tgbot::LogLevel::Debug);

    log.info("=== tgbot - Teste Core (Fase 1, Passo 1.2) ===");

    tgbot::Randomizer rng;
    const cv::Rect fakeButton(500, 300, 180, 60);

    for (int i = 0; i < 5; ++i) {
        const cv::Point p = rng.tapPointInBox(fakeButton);
        const auto wait = rng.sleepDuration(800, 1400);
        log.debug("Toque simulado #" + std::to_string(i + 1) +
                  " -> (" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")" +
                  " | Espera: " + std::to_string(wait.count()) + " ms");
    }

    log.info("Logger e Randomizer validados com sucesso.");
    return 0;
}