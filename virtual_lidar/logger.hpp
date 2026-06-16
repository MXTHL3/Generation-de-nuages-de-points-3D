#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

/// @brief initialisation Logger spdlog globale
/// Doit être appelé une seule fois au démarrage (dans main pour l'instant)
/// Le niveau de log par défaut est "TRACE" < "INFO" < "DEBUG" < ...

class Logger{
public:
    static void init(){
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        auto logger = std::make_shared<spdlog::logger>("LOGGER", console_sink);

        logger->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
        
        logger->set_level(spdlog::level::trace);
        //logger->set_level(spdlog::level::info);

        spdlog::set_default_logger(logger);
    }
};


// Macro de niveau de logs
#define SIM_INFO(...) spdlog::info(__VA_ARGS__)
#define SIM_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define SIM_WARNING(...) spdlog::warn(__VA_ARGS__)
#define SIM_ERROR(...) spdlog::error(__VA_ARGS__)
 
#endif