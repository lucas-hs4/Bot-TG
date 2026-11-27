#include "core/Randomizer.hpp"

#include <algorithm>

namespace tgbot {

Randomizer::Randomizer()
    : engine_(std::random_device{}()) {}

int Randomizer::rangeInt(int minInclusive, int maxInclusive) {
    if (minInclusive >= maxInclusive) {
        return minInclusive;
    }
    std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
    return dist(engine_);
}

std::chrono::milliseconds Randomizer:: sleepDuration(int minMs, int maxMs) {
    return std::chrono::milliseconds(rangeInt(minMs, maxMs));
}

cv::Point Randomizer::tapPointInBox(const cv::Rect& box, double marginRatio) {
    const double clampedMargin = std::clamp(marginRatio, 0.0, 0.45);
    const int marginX = static_cast<int>(box.width * clampedMargin);
    const int marginY = static_cast<int>(box.height * clampedMargin);

    const int minX = box.x + marginX;
    const int maxX = (std::max)(minX, box.x + box.width - 1- marginX);

    const int minY = box.y + marginY;
    const int maxY = (std::max)(minY, box.y + box.height - 1 - marginY);

    return cv::Point(rangeInt(minX, maxX), rangeInt(minY, maxY));
}

}