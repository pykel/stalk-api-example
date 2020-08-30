#include "logger.h"
#include <vector>
#include "spdlog/sinks/stdout_sinks.h"

namespace
{

class LoggerImpl
{
public:

    LoggerImpl()
        // default pattern is "%+"
//        sinks_({
//                 //std::make_shared<spdlog::sinks::rotating_file_sink_mt> ("logs/logfile", 1024*1024*5, 10),
//                 std::make_shared<spdlog::sinks::stdout_sink_mt>()
//               })
    {
        auto formatter = std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%dT%T.%eZ][%n][%l] %v", spdlog::pattern_time_type::utc);
        spdlog::set_formatter(std::move(formatter));
        sinks_ = {
                 //std::make_shared<spdlog::sinks::rotating_file_sink_mt> ("logs/logfile", 1024*1024*5, 10),
                 std::make_shared<spdlog::sinks::stdout_sink_mt>()
               };

    }

    std::shared_ptr<spdlog::logger> get(const std::string& name)
    {
        auto logger = spdlog::get(name);
        if (!logger)
        {
            logger = std::make_shared<spdlog::logger>(name, std::begin(sinks_), std::end(sinks_));
            //logger->set_formatter(formatter_);
            logger->set_level(level_);
            //spdlog::register_logger(logger);  // don't register just in case 1000s created that are never 'dropped'
        }
        return logger;
    }

    void setLevel(spdlog::level::level_enum level, const std::string& name = std::string())
    {
        if (!name.empty())
        {
            auto logger = spdlog::get(name);
            if (logger)
                logger->set_level(level);
        }
        else
        {
            level_ = level;
            spdlog::set_level(level);
        }
    }

private:
    std::vector<spdlog::sink_ptr> sinks_;
    spdlog::level::level_enum level_;
};

} // anon namespace

namespace Logger
{

std::unique_ptr<LoggerImpl> impl;

LoggerImpl& loggerInstance()
{
    if (!impl)
        impl = std::make_unique<LoggerImpl>();

    return *impl;
}

std::shared_ptr<spdlog::logger> get(const std::string& name)
{
    return loggerInstance().get(name);
}

void setLevel(spdlog::level::level_enum lvl, const std::string& name)
{
    loggerInstance().setLevel(lvl, name);
}

}
