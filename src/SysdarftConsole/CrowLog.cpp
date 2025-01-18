#include <SysdarftMain.h>

void RemoteDebugServer::SysdarftLogHandler::log(std::string message, crow::LogLevel level)
{
    if (!log_available) {
        return;
    }

    const char *level_text = nullptr;
    switch (level) {
    case crow::LogLevel::Debug:
        level_text = "[DEBUG]";
        break;
    case crow::LogLevel::Info:
        level_text = "[INFO]";
        break;
    case crow::LogLevel::Warning:
        level_text = "[WARN]";
        break;
    case crow::LogLevel::Error:
        level_text = "[ERROR]";
        break;
    case crow::LogLevel::Critical:
        level_text = "[CRITICAL]";
        break;
    default:
        level_text = "[LOG]";
        break;
    }

    custom_stream << level_text << " " << message << std::endl;
}
