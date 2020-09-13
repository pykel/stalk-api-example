#include "logger.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include "spdlog/sinks/stdout_sinks.h"

namespace
{

class LoggerImpl
{
public:

    LoggerImpl()
    {
        auto formatter = std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%dT%T.%eZ][%n][%l] %v", spdlog::pattern_time_type::utc);
        spdlog::set_formatter(std::move(formatter));
        sinks_ = {
            //std::make_shared<spdlog::sinks::rotating_file_sink_mt> ("logs/logfile", 1024*1024*5, 10),
            std::make_shared<spdlog::sinks::stdout_sink_mt>()
        };
    }

    std::shared_ptr<Logger> get(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(logger_map_mutex_);

        auto found = loggers_.find(name);
        if (found != loggers_.end())
            return found->second;

        auto logger = std::make_shared<Logger>(name, std::begin(sinks_), std::end(sinks_));
        //logger->set_formatter(formatter_);
        logger->set_level(level_);
        //spdlog::register_logger(logger);  // don't register just in case 1000s created that are never 'dropped'
        loggers_[name] = logger;
        return logger;
    }

    void drop(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(logger_map_mutex_);
        loggers_.erase(name);
    }

    void setLevel(spdlog::level::level_enum level, const std::string& name = std::string())
    {        
        if (!name.empty())
        {
            std::lock_guard<std::mutex> lock(logger_map_mutex_);

            auto found = loggers_.find(name);
            if (found != loggers_.end())
            {
                found->second->set_level(level);
            }
        }
        else
        {
            level_ = level;
            spdlog::set_level(level);
        }
    }

    std::unordered_map<std::string, std::shared_ptr<Logger>> loggers() const
    {
        std::lock_guard<std::mutex> lock(logger_map_mutex_);
        return loggers_;
    }

private:
    std::vector<spdlog::sink_ptr> sinks_;
    spdlog::level::level_enum level_;
    mutable std::mutex logger_map_mutex_;
    std::unordered_map<std::string, std::shared_ptr<Logger>> loggers_;
};


LoggerImpl& loggerInstance()
{
    static std::unique_ptr<LoggerImpl> impl;
    if (!impl)
        impl = std::make_unique<LoggerImpl>();

    return *impl;
}

} // anon namespace


Logger::~Logger()
{
    loggerInstance().drop(name_);
}

std::shared_ptr<Logger> Logger::get(const std::string& name)
{
    return loggerInstance().get(name);
}

void Logger::setLevel(Level lvl, const std::string& name)
{
    loggerInstance().setLevel(lvl, name);
}

std::unordered_map<std::string, std::shared_ptr<Logger>> Logger::loggers()
{
    return loggerInstance().loggers();
}

