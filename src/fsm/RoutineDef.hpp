#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <opencv2/core/types.hpp>

namespace tgbot {

enum class ActionType {
    TapDetected,
    TapPoint,
    Swipe,
    Wait,
    KeyBack,
    Finish,
};

struct ActionDef {
    std::string ifTemplate;
    double threshold = -1.0;
    cv::Rect roi;

    ActionType type = ActionType::TapDetected;
    std::string targetTemplate;
    cv::Point tapPoint;
    cv::Point swipeFrom;
    cv::Point swipeTo;
    int swipeDurationMs = 350;

    int waitMinMs = 800;
    int waitMaxMs = 1400;
};

struct StateDef {
    std::string id;
    std::string detectTemplate;
    double threshold = -1.0;
    cv::Rect roi;
    std::vector<ActionDef> actions;
};

struct RoutineDef {
    std::string name;
    double defaultThreshold = 0.85;
    int loopIntervalMs = 600;
    int stuckTimeoutSec = 45;
    std::vector<StateDef> globalHandlers;
    std::vector<StateDef> states;
};

std::optional<RoutineDef> loadRoutineFromFile(const std::filesystem::path& path);

}