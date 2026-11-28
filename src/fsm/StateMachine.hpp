#pragma once

#include <atomic>
#include <string>

#include <opencv2/core/mat.hpp>

#include "fsm/RoutineDef.hpp"

namespace tgbot {

class AdbController;
class Randomizer;
class ScreenDetector;

class StateMachine {
public:
    StateMachine(AdbController& adb, ScreenDetector& detector, Randomizer& rng);

    bool run(const RoutineDef& routine, int maxIterations = 0);
    void requestStop() noexcept;

private:
    enum class ExecStatus {
        NoAction,
        ActionExecuted,
        Finished
    };

    const StateDef* matchAnyState(const cv::Mat& frame,
                                  const std::vector<StateDef>& candidates,
                                  double defaultThreshold,
                                  cv::Rect& outMatchedBox) const;
    
    ExecStatus executeStateActions(const cv::Mat& frame,
                                   const StateDef& state,
                                   const cv::Rect& stateBox,
                                   double defaultThreshold);
    
    AdbController& adb_;
    ScreenDetector& detector_;
    Randomizer& rng_;
    std::atomic<bool> stopRequested_{false};
};

}