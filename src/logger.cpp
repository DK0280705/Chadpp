#include "logger.h"
#include <dpp/fmt/format.h>
#include <dpp/utility.h>

static void log(const std::string& msg)
{
    std::string timestamp = dpp::utility::current_date_time();
    fmt::print("{}: {}\n", timestamp, msg);
}

void log_info(const char* label, const std::string& msg)
{
    log(fmt::format("\033[1;32m[INFO]\033[0m [{}] {}", label, msg));
}

void log_debug(const char* label, const std::string& msg)
{
    log(fmt::format("\033[1;34m[DEBUG]\033[0m [{}] {}", label, msg));
}

void log_warn(const char* label, const std::string& msg)
{
    log(fmt::format("\033[1;33m[WARN]\033[0m [{}] {}", label, msg));
}

void log_err(const char* label, const std::string& msg)
{
    log(fmt::format("\033[1;31m[ERROR]\033[0m [{}] {}", label, msg));
}

void log_crit(const char* label, const std::string& msg)
{
    log(fmt::format("\033[1;41;30m[CRITICAL]\033[2;41;30m [{}] {}\033[0m", label, msg));
}