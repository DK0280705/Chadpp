#include "command.h"

Command::Command(const char* name,
                 const int desc_id,
                 const std::vector<dpp::command_option>& opts)
    : name(name), desc_id(desc_id), options(opts)
{
}

Command::~Command() {}
