#include "vision/ScreenDetector.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "core/Logger.hpp"

namespace tgbot {

bool ScreenDetector::loadTemplatesFromDir(const std::filesystem::path& rootDir) {
    templates_.clear();

    if (!std::filesystem::exists(rootDir)) {
        Logger::instance().warn("Diretorio de templates nao encontrado: " + rootDir.string());
        return false;
    }
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(rootDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".png") {
            continue;
        }

        cv::Mat img = cv::imread(entry.path().string(), cv::IMREAD_COLOR);
        if (img.empty()) {
            Logger::instance().warn("Falha ao carregar template PNG: " + entry.path().string());
            continue;
        }

        const std::string key = std::filesystem::relative(entry.path(), rootDir).generic_string();
        templates_.emplace(key, std::move(img));
    }

    Logger::instance().info("Templates carregados em cache RAM: " + std::to_string(templates_.size()));
    return true;
}

bool ScreenDetector::hasTemplate(const std::string& name) const {
    return templates_.find(name) != templates_.end();
}

MatchResult ScreenDetector::findTemplate(const cv::Mat& frame,
                                         const std::string& templateName,
                                         double threshold,
                                         const cv::Rect& roi) const {
    if (frame.empty()) {
        return {};
    }

    const auto it = templates_.find(templateName);
    if (it == templates_.end()) {
        Logger::instance().warn("Template nao encontrado no cache: " + templateName);
        return {};
    }

    const cv::Mat& templ = it->second;
    const cv::Rect frameBounds(0, 0, frame.cols, frame.rows);
    const cv::Rect safeRoi = (roi.width > 0 && roi.height > 0)
                                 ? (roi & frameBounds)
                                 : frameBounds;
    if (safeRoi.width < templ.cols || safeRoi.height < templ.rows) {
        return {};
    }

    const cv::Mat searchArea = frame(safeRoi);

    cv::Mat result;
    cv::matchTemplate(searchArea, templ, result, cv::TM_CCOEFF_NORMED);

    double maxVal = 0.0;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    MatchResult match;
    match.confidence = maxVal;
    match.found = (maxVal >= threshold);
    match.box = cv::Rect(safeRoi.x + maxLoc.x, safeRoi.y + maxLoc.y, templ.cols, templ.rows);
    return match;
}

}