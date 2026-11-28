#include "core/AdbController.hpp"

#include <windows.h>

#include <array>
#include <utility>

#include <opencv2/imgcodecs.hpp>

#include "core/Logger.hpp"
#include "core/Randomizer.hpp"

namespace tgbot {

namespace {

class UniqueHandle {
public:
    UniqueHandle() = default;
    explicit UniqueHandle(HANDLE handle) noexcept : handle_(handle) {}

    ~UniqueHandle() { reset(); }

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    UniqueHandle(UniqueHandle&& other) noexcept
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    UniqueHandle& operator=(UniqueHandle&& other) noexcept {
        if (this != &other) {
            reset(std::exchange(other.handle_, nullptr));
        }
        return *this;
    }

    void reset(HANDLE newHandle = nullptr) noexcept {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
        handle_ = newHandle;
    }

    HANDLE get() const noexcept { return handle_; }

private:
    HANDLE handle_ = nullptr;
};

std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) {
        return {};
    }
    const int sizeNeeded = MultiByteToWideChar(
        CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring result(sizeNeeded, L'\0');
    MultiByteToWideChar(
        CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded);
    return result;
}

}

AdbController::AdbController(std::string adbPath, std::string deviceId)
    : adbPath_(std::move(adbPath)), deviceId_(std::move(deviceId)) {}

std::string AdbController::buildDevicePrefix() const {
    if (deviceId_.empty()) {
        return {};
    }
    return "-s " + deviceId_ + " ";
}

bool AdbController::runCommand(const std::string& args,
                               std::vector<std::uint8_t>& outBytes,
                               unsigned long timeoutMs) const {
    outBytes.clear();

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE rawRead = nullptr;
    HANDLE rawWrite = nullptr;
    if (!CreatePipe(&rawRead, &rawWrite, &sa, 0)) {
        Logger::instance().error("CreatePipe falhou ao executar ADB.");
        return false;
    }

    UniqueHandle readPipe(rawRead);
    UniqueHandle writePipe(rawWrite);

    if (!SetHandleInformation(readPipe.get(), HANDLE_FLAG_INHERIT, 0)) {
        Logger::instance().error("SetHandleInformation falhou.");
        return false;
    }

    STARTUPINFOW si{};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = writePipe.get();
    si.hStdError = nullptr;
    si.hStdInput = nullptr;

    PROCESS_INFORMATION pi{};
    std::wstring cmdLine = utf8ToWide("\"" + adbPath_ + "\" " + buildDevicePrefix() + args);

    const BOOL created = CreateProcessW(
        nullptr,
        cmdLine.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi);
    
    if (!created) {
        Logger::instance().error("CreateProcessW falhou ao iniciar adb.exe (codigo Win32: " + 
                                 std::to_string(GetLastError()) + ").");
        return false;
    }

    UniqueHandle procHandle(pi.hProcess);
    UniqueHandle threadHandle(pi.hThread);

    writePipe.reset();


    std::array<std::uint8_t, 65536> chunk{};
    DWORD bytesRead = 0;
    while (ReadFile(readPipe.get(), chunk.data(), static_cast<DWORD>(chunk.size()), &bytesRead, nullptr) && 
           bytesRead > 0) {
        outBytes.insert(outBytes.end(), chunk.data(), chunk.data() + bytesRead);
    }

    const DWORD waitResult = WaitForSingleObject(procHandle.get(), timeoutMs);
    if (waitResult != WAIT_OBJECT_0) {
        Logger::instance().error("Timeout aguardando processo ADB.");
        TerminateProcess(procHandle.get(), 1);
        return false;
    }

    DWORD exitCode = 1;
    GetExitCodeProcess(procHandle.get(), &exitCode);
    return exitCode == 0;
}

bool AdbController::checkConnection() const {
    std::vector<std::uint8_t> output;
    if (!runCommand("get-state", output, 5000)) {
        return false;
    }
    const std::string state(output.begin(), output.end());
    return state.find("device") != std::string::npos;
}

cv::Mat AdbController::captureScreen() const {
    std::vector<std::uint8_t> pngBuffer;
    pngBuffer.reserve(1536 * 1024);

    if (!runCommand("exec-out screencap -p", pngBuffer, 10000) || pngBuffer.empty()) {
        Logger::instance().error("Falha ao capturar stdout de screencap via ADB.");
        return {};
    }
    
    cv::Mat frame = cv::imdecode(pngBuffer, cv::IMREAD_COLOR);
    if (frame.empty()) {
        Logger::instance().error("cv::imdecode falhou (buffer PNG invalido ou corrompido).");
    }
    return frame;
}

bool AdbController::tap(int x, int y) const {
    std::vector<std::uint8_t> ignored;
    return runCommand("shell input tap " + std::to_string(x) + " " + std::to_string(y), ignored, 5000);
}

bool AdbController::tapInBox(const cv::Rect& box, Randomizer& rng) const {
    const cv::Point pt = rng.tapPointInBox(box);
    Logger::instance().debug("ADB tap -> (" + std::to_string(pt.x) + ", " + std::to_string(pt.y) + ")");
    return tap(pt.x, pt.y);
}

bool AdbController::swipe(cv::Point from, cv::Point to, int durationMs) const {
    std::vector<std::uint8_t> ignored;
    const std::string cmd = "shell input swipe " +
                            std::to_string(from.x) + " " + std::to_string(from.y) + " " +
                            std::to_string(to.x) + " " + std::to_string(to.y) + " " +
                            std::to_string(durationMs);
    return runCommand(cmd, ignored, 5000);
}

bool AdbController::keyEvent(int keyCode) const {
    std::vector<std::uint8_t> ignored;
    return runCommand("shell input keyevent " + std::to_string(keyCode), ignored, 5000);
}

}