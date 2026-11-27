#include "core/Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace tgbot {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    minLevel_ = level;
}

void Logger::enableFileOutput(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    fileStream_.open(filePath, std::ios::app);
}

void Logger::log(LogLevel level, std::string_view message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (static_cast<int>(level) < static_cast<int>(minLevel_)) {
        return;
    }

    const std::string line = "[" + currentTimestamp() + "] [" + levelToString(level) + "] " + std::string(message);

    if (level == LogLevel::Error) {
        std::cerr << line << '\n';
    } else {
        std::cout << line << '\n';
    }

    if (fileStream_.is_open()) {
        fileStream_ << line << '\n';
        fileStream_.flush();
    }
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "INFO";
}

std::string Logger::currentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (now.time_since_epoch()) % 1000;
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTm{};
    localtime_s(&localTm, &nowTime);

    std::ostringstream oss;
    oss << std::put_time(&localTm, "%H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

}