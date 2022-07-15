#include "../module.h"
#include "commands.h"
#include <dlfcn.h>
#include <dpp/fmt/format.h>

Command_reload::Command_reload()
    : Command("reload",
              COMMAND_RELOAD_DESC,
              {{dpp::command_option(dpp::co_string, "name", "binary name", true)}})
{
    owner_only = true;
    category   = cat_owner;
}

void Command_reload::call(const Input& input) const
{
    const std::string& name = std::get<std::string>(input[0]);

    const bool succeed = reload_module(&Bot::instance(), name.c_str());

    if (!succeed) input.reply(dlerror());
    else input.reply("Reloaded");
}
