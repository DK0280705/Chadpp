#include "commands.h"
#include "dpp/cluster.h"
#include <dpp/appcommand.h>

Command_publish::Command_publish()
    : Command("publish", COMMAND_PUBLISH_DESC,
              {{dpp::command_option(dpp::co_string, "cmd", "", true)},
               {dpp::command_option(dpp::co_string, "scmd", "")}})
{
    owner_only = true;
    category   = cat_owner;
}

void Command_publish::call(const Input& input) const
{
    const std::string& cmd = std::get<std::string>(input[0]);
    if (cmd == "add") {
        bot->global_bulk_command_create_sync(bot->build_slashcommands());
        input.reply("Published");
    } else if (cmd == "remove") {
        bot->global_commands_get([input](const dpp::confirmation_callback_t& cb) {
            const std::string& cmd = std::get<std::string>(input[1]);
            const dpp::slashcommand_map& map = std::get<dpp::slashcommand_map>(cb.value);
            const auto it = std::find_if(map.begin(), map.end(), [&](const auto& sc) {
                return sc.second.name == cmd;
            });
            if (it == map.end()) return input.reply("bruh");
            bot->global_command_delete(it->first);
        });
    } else input.reply("bruh");
}