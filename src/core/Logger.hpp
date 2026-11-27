#pragma once

#include <fstream>
#include <mutex>
#include <string>
#include <string_view>

namespace tgbot {

enum class LogLevel {
    Debug = 0,
    Info,
    Warn,
    Error,
};

class Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level);
    void enableFileOutput(const std::string& filePath);

    void log(LogLevel level, std::string_view message);

    void debug(std::string_view message) { log(LogLevel::Debug, message); }
    void info(std::string_view message) { log(LogLevel::Info, message); }
    void warn(std::string_view message) { log(LogLevel::Warn, message); }
    void error(std::string_view message) { log(LogLevel::Error, message); }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;

    static const char* levelToString(LogLevel level);
    static std::string currentTimestamp();

    std::mutex mutex_;
    LogLevel minLevel_ = LogLevel::Info;
    std::ofstream fileStream_;
};

}