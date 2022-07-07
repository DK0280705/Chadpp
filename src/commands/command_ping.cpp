#include "commands.h"
#include <dpp/utility.h>

Command_ping::Command_ping() : Command("ping", COMMAND_PING_DESC, {})
{
    category = cat_general;
}

void Command_ping::call(const Input& input) const
{
    const double now  = dpp::utility::time_f();
    const double ping = bot->rest_ping * 1000;
    input.reply_sync("Wait...");
    const double end = dpp::utility::time_f();
    input.edit_reply("```Rest Latency: " + std::to_string(ping) +
                     "ms\nBot Latency: " + std::to_string((end - now) * 1000) + "ms```");
}
