#pragma once
#include <string>

void log_info(const char* label, const std::string& msg);

void log_debug(const char* label, const std::string& msg);

void log_warn(const char* label, const std::string& msg);

void log_err(const char* label, const std::string& msg);

void log_crit(const char* label, const std::string& msg);