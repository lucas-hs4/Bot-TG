#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <opencv2/core.hpp>

namespace tgbot {

struct MatchResult {
    bool found= false;
    double confidence = 0.0;
    cv::Rect box;
};

class ScreenDetector {
public:
    bool loadTemplatesFromDir(const std::filesystem::path& rootDir);
    bool hasTemplate(const std::string& name) const;
    std::size_t templateCount() const noexcept { return templates_.size(); }

    MatchResult findTemplate(const cv::Mat& frame,
                             const std::string& templateName,
                             double threshold = 0.85,
                             const cv::Rect& roi = cv::Rect()) const;
private:
    std::unordered_map<std::string, cv::Mat> templates_;
};

}