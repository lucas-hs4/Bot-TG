#include "fsm/StateMachine.hpp"

#include <chrono>
#include <thread>

#include "core/AdbController.hpp"
#include "core/Logger.hpp"
#include "core/Randomizer.hpp"
#include "vision/ScreenDetector.hpp"

namespace tgbot {

StateMachine::StateMachine(AdbController& adb, ScreenDetector& detector, Randomizer& rng)
    : adb_(adb), detector_(detector), rng_(rng) {}

void StateMachine::requestStop() noexcept {
    stopRequested_.store(true);
}

const StateDef* StateMachine::matchAnyState(const cv::Mat& frame,
                                            const std::vector<StateDef>& candidates,
                                            double defaultThreshold,
                                            cv::Rect& outMatchedBox) const {
    for (const auto& state : candidates) {
        if (state.detectTemplate.empty()) {
            continue;
        }
        const double threshold = (state.threshold >= 0.0) ? state.threshold : defaultThreshold;
        const MatchResult res = detector_.findTemplate(frame, state.detectTemplate, threshold, state.roi);
        if (res.found) {
            outMatchedBox = res.box;
            return &state;
        }
    }
    return nullptr;
}

StateMachine::ExecStatus StateMachine::executeStateActions(const cv::Mat& frame,
                                                           const StateDef& state,
                                                           const cv::Rect& stateBox,
                                                           double defaultThreshold) {
    for (const auto& act : state.actions) {
        cv::Rect actionBox = stateBox;

        if (!act.ifTemplate.empty()) {
            const double th = (act.threshold >= 0.0) ? act.threshold : defaultThreshold;
            const MatchResult cond = detector_.findTemplate(frame, act.ifTemplate, th, act.roi);
            if (!cond.found) {
                continue;
            }
            actionBox = cond.box;
        }

        switch (act.type) {
            case ActionType::TapDetected: {
                if (!act.targetTemplate.empty()) {
                    const double th = (act.threshold >= 0.0) ? act.threshold : defaultThreshold;
                    const MatchResult target = detector_.findTemplate(frame, act.targetTemplate, th, act.roi);
                    if (!target.found) {
                        continue;
                    }
                    actionBox = target.box;
                }
                adb_.tapInBox(actionBox, rng_);
                break;
            }
            case ActionType::TapPoint: {
                const cv::Rect safeBox(act.tapPoint.x - 5, act.tapPoint.y - 5, 10, 10);
                adb_.tapInBox(safeBox, rng_);
                break;
            }
            case ActionType::Wait:
                break;
            case ActionType::KeyBack:
                adb_.keyEvent(4);
                break;
            case ActionType::Finish:
                Logger::instance().info("Acao 'finish' alcancada no estado: " + state.id);
                return ExecStatus::Finished;
        }

        std::this_thread::sleep_for(rng_.sleepDuration(act.waitMinMs, act.waitMaxMs));
        return ExecStatus::ActionExecuted;
    }

    return ExecStatus::NoAction;
}

bool StateMachine::run(const RoutineDef& routine, int maxIterations) {
    auto& log = Logger::instance();
    log.info("Iniciando FSM para rotina: " + routine.name);

    stopRequested_.store(false);
    int iteration = 0;
    int consecutiveCaptureFailures = 0;
    bool recoveryAttempted = false;

    std::string lastStateId = "NONE";
    auto lastProgressTime = std::chrono::steady_clock::now();

    while (!stopRequested_.load()) {
        if (maxIterations > 0 && iteration >= maxIterations) {
            log.info("Limite de iteracoes de teste atigindo (" + std::to_string(maxIterations) + ").");
            return true;
        }
        ++iteration;

        const cv::Mat frame = adb_.captureScreen();
        if (frame.empty()) {
            ++consecutiveCaptureFailures;
            log.warn("Captura vazia (" + std::to_string(consecutiveCaptureFailures) + "/5).");
            if (consecutiveCaptureFailures >= 5) {
                log.error("Muitas falhas consecutivas de captura ADB. Abortando FSM.");
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        consecutiveCaptureFailures = 0;

        cv::Rect matchedBox;
        const StateDef* activeState = matchAnyState(frame, routine.globalHandlers, routine.defaultThreshold, matchedBox);
        if (activeState == nullptr) {
            activeState = matchAnyState(frame, routine.states, routine.defaultThreshold, matchedBox);
        }

        const std::string currentStateId = activeState ? activeState->id : "UNKNOWN";
        if (currentStateId != lastStateId) {
            log.info("Estado: " + lastStateId + " -> " + currentStateId);
            lastStateId = currentStateId;
            lastProgressTime = std::chrono::steady_clock::now();
            recoveryAttempted = false;
        }

        if (activeState != nullptr) {
            const ExecStatus status = executeStateActions(
                frame, *activeState, matchedBox, routine.defaultThreshold);
            
            if (status == ExecStatus::Finished) {
                return true;
            }
            if (status == ExecStatus::ActionExecuted) {
                lastProgressTime = std::chrono::steady_clock::now();
                recoveryAttempted = false;
                continue;
            }
        }

        const auto stuckSec = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - lastProgressTime).count();
        
        if (stuckSec >= routine.stuckTimeoutSec) {
            if (!recoveryAttempted) {
                log.warn("Watchdog: sem progresso ha " + std::to_string(stuckSec) + 
                         "s no estado " + currentStateId + ". Tentando KEYCODE_BACK.");
                adb_.keyEvent(4);
                recoveryAttempted = true;
                lastProgressTime = std::chrono::steady_clock::now();
            } else {
                log.error("Watchdog: bot travado no estado " + currentStateId + " mesmo apos recuperacao.");
                return false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(routine.loopIntervalMs));
    }

    log.info("FSM interrompida por solicitacao do usuario.");
    return true;
}

}