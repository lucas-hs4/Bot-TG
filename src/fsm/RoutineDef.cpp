#include "fsm/RoutineDef.hpp"

#include <nlohmann/json.hpp>

#include "core/ConfigLoader.hpp"
#include "core/Logger.hpp"

namespace tgbot{

namespace {

cv::Rect parseRoi(const nlohmann::json& j) {
    if (!j.contains("roi") || !j["roi"].is_array() || j["roi"].size() != 4) {
        return {};
    }
    const auto& r = j["roi"];
    return cv::Rect(r[0].get<int>(), r[1].get<int>(), r[2].get<int>(), r[3].get<int>());
}

cv::Point parsePoint(const nlohmann::json& j, const char* key) {
    if (!j.contains(key) || !j[key].is_array() || j[key].size() != 2) {
        return {};
    }
    const auto& p = j[key];
    return cv::Point(p[0].get<int>(), p[1].get<int>());
}

ActionType parseActionType(const std::string& typeStr) {
    if (typeStr == "tap")       return ActionType::TapDetected;
    if (typeStr == "tap_point") return ActionType::TapPoint;
    if (typeStr == "swipe")     return ActionType::Swipe;
    if (typeStr == "wait")      return ActionType::Wait;
    if (typeStr == "key_back")  return ActionType::KeyBack;
    if (typeStr == "finish")    return ActionType::Finish;

    Logger::instance().warn("Tipo de acao desconhecido '" + typeStr + "', usando 'tap'.");
    return ActionType::TapDetected;
}

ActionDef parseAction(const nlohmann::json& j) {
    ActionDef act;
    act.ifTemplate = j.value("if_template", "");
    act.threshold = j.value("threshold", -1.0);
    act.roi = parseRoi(j);
    act.type = parseActionType(j.value("action", "tap"));
    act.targetTemplate = j.value("target_template", "");
    act.tapPoint = parsePoint(j, "point");
    act.swipeFrom = parsePoint(j, "from");
    act.swipeTo = parsePoint(j, "to");
    act.swipeDurationMs = j.value("duration_ms", 350);

    if (j.contains("wait_ms") && j["wait_ms"].is_array() && j["wait_ms"].size() == 2) {
        act.waitMinMs = j["wait_ms"][0].get<int>();
        act.waitMaxMs = j["wait_ms"][1].get<int>();
    }
    return act;
}

StateDef parseState(const nlohmann::json& j) {
    StateDef st;
    st.id = j.at("id").get<std::string>();
    st.detectTemplate = j.at("detect_template").get<std::string>();
    st.threshold = j.value("threshold", -1.0);
    st.roi = parseRoi(j);

    if (j.contains("actions") && j["actions"].is_array()) {
        for (const auto& actJson : j["actions"]) {
            st.actions.push_back(parseAction(actJson));
        }
    }
    return st;
}

}

std::optional<RoutineDef> loadRoutineFromFile(const std::filesystem::path& path) {
    const auto jsonOpt = ConfigLoader::loadJsonFile(path);
    if (!jsonOpt.has_value()) {
        return std::nullopt;
    }

    const auto& j = *jsonOpt;
    try {
        RoutineDef routine;
        routine.name = j.at("routine_name").get<std::string>();
        routine.defaultThreshold = j.value("default_threshold", 0.85);
        routine.loopIntervalMs = j.value("loop_interval_ms", 600);
        routine.stuckTimeoutSec = j.value("stuck_timeout_sec", 45);

        if (j.contains("global_handlers") && j["global_handlers"].is_array()) {
            for (const auto& hJson : j["global_handlers"]) {
                routine.globalHandlers.push_back(parseState(hJson));
            }
        }

        for (const auto& sJson : j.at("states")) {
            routine.states.push_back(parseState(sJson));
        }

        return routine;
    } catch (const nlohmann::json::exception& e) {
        Logger::instance().error("Erro na estrutura da rotina (" + path.string() + "): " + e.what());
        return std::nullopt;
    }
}

}