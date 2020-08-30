#ifndef logger_INCLUDED
#define logger_INCLUDED

#include <string>
#include <memory>
#include <vector>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_sinks.h"


namespace Logger
{

std::shared_ptr<spdlog::logger> get(const std::string& name);

void setLevel(spdlog::level::level_enum lvl, const std::string& name = std::string());

typedef std::shared_ptr<spdlog::logger> Log;

} // namespace Logger

//-------------------------------------------------------



#endif // ifndef logger_INCLUDED
