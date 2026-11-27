#pragma once

#include <chrono>
#include <random>

#include <opencv2/core/types.hpp>

namespace tgbot {

class Randomizer {
public:
    Randomizer();

    int rangeInt(int minInclusive, int maxInclusive);
    std::chrono::milliseconds sleepDuration(int minMs, int maxMs);

    cv::Point tapPointInBox(const cv::Rect& box, double marginRatio = 0.15);

private:
    std::mt19937 engine_;
};

}