#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class Logger{
public:
    static void init(){
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("LOGGER", console_sink);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        logger->set_level(spdlog::level::info);
        logger->set_level(spdlog::level::debug);
        spdlog::set_default_logger(logger);
    }
};


#define SIM_INFO(...) spdlog::info(__VA_ARGS__)
#define SIM_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define SIM_WARNING(...) spdlog::warn(__VA_ARGS__)
#define SIM_ERROR(...) spdlog::error(_VA_ARGS__)