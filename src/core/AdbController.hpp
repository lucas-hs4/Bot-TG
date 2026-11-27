#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

namespace tgbot {

class Randomizer;

class AdbController {
public:
    AdbController(std::string adbPath, std::string deviceId);

    bool checkConnection() const;
    cv::Mat captureScreen() const;

    bool tap(int x, int y) const;
    bool tapInBox(const cv::Rect& box, Randomizer& rng) const;
    bool swipe(cv::Point from, cv::Point to, int durationMs) const;

    const std::string& deviceId() const noexcept { return deviceId_; }

private:
    bool runCommand(const std::string& args,
                    std::vector<std::uint8_t>& outBytes,
                    unsigned long timeoutMs = 10000) const;

    std::string buildDevicePrefix() const;

    std::string adbPath_;
    std::string deviceId_;
};

}