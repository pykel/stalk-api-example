#ifndef logger_INCLUDED
#define logger_INCLUDED

#include <string>
#include <memory>
#include <mutex>
#include <vector>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_sinks.h"

//-------------------------------------------------------

class Logger : public std::enable_shared_from_this<Logger>, public spdlog::logger
{
public:

    using Level = spdlog::level::level_enum;

    template<typename It>
    Logger(std::string name, It begin, It end) : spdlog::logger(name, begin, end) {}

    ~Logger();
    static std::shared_ptr<Logger> get(const std::string& name);
    static std::unordered_map<std::string, std::shared_ptr<Logger>> loggers();

    static void setLevel(Level lvl, const std::string& name = std::string());
};

using Log = std::shared_ptr<Logger>;
using LogPtr = std::shared_ptr<Logger>;

#endif // ifndef logger_INCLUDED

