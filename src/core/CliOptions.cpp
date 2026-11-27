#include "core/CliOptions.hpp"

#include <iostream>
#include <string_view>

#include "core/Logger.hpp"

namespace tgbot {

namespace {

std::string resolveRoutinePath(std::string_view folder, std::string_view value) {
    if (value.ends_with(".json")) {
        return std::string(value);
    }
    return "config/" + std::string(folder) + "/" + std::string(value) + ".json";
}

}

void printCliHelp() {
    std::cout << "Uso: tgbot.exe [opcoes]\n\n"
              << "Opcoes:\n"
              << " --routine <nome|caminho>   Rotina em config/routines/<nome>.json (padrao: dailies)\n"
              << " --event <nome|caminho>     Evento em config/events/<nome>.json\n"
              << " --device <id>              Sobrescreve device_id do device.json\n"
              << " --config <caminho>         Caminho do device.json (padrao: config/device.json)\n"
              << " --debug                    Ativa logs em nivel DEBUG\n"
              << " -h, --help                 Exibe esta ajuda\n";
}

std::optional<CliOptions> parseCliArgs(int argc, char* argv[]) {
    CliOptions opts;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);

        if (arg == "-h" || arg == "--help") {
            opts.showHelp = true;
            return opts;
        }
        if (arg == "--debug") {
            opts.debug = true;
            continue;
        }

        auto nextValue = [&](std::string_view flag) -> std::optional<std::string_view> {
            if (i + 1 >= argc) {
                Logger::instance().error("Faltou valor apos o argumento: " + std::string(flag));
                return std::nullopt;
            }
            return std::string_view(argv[++i]);
        };

        if (arg == "--routine") {
            const auto val = nextValue(arg);
            if (!val) return std::nullopt;
            opts.routinePath = resolveRoutinePath("routines", *val);
        } else if (arg == "--event") {
            const auto val = nextValue(arg);
            if (!val) return std::nullopt;
            opts.routinePath = resolveRoutinePath("events", *val);
        } else if (arg == "--device") {
            const auto val = nextValue(arg);
            if (!val) return std::nullopt;
            opts.deviceOverride = std::string(*val);
        } else if (arg == "--config") {
            const auto val = nextValue(arg);
            if (!val) return std::nullopt;
            opts.configPath = std::string(*val);
        } else {
            Logger::instance().error("Argumento desconhecido: " + std::string(arg));
            return std::nullopt;
        }
    }

    return opts;
}

}