#include "../database.h"
#include "commands.h"
#include <pqxx/pqxx>

Command_afk::Command_afk()
    : Command("afk", COMMAND_AFK_DESC, {{dpp::command_option(dpp::co_string, "reason", "")}})
{
    category = cat_general;
}

void Command_afk::call(const Input& input) const
{
    const std::string reason = input->args.empty()
                                   ? "Just afk"
                                   : bot->database.conn->esc(std::get<std::string>(input[0]));
    bot->database.execute_sync("INSERT INTO chadpp.afk_users (id, reason) VALUES (" +
                               pqxx::to_string(input->author.id) + ", '" + reason + "')");
    input.reply(_(bot->g_lang(input->guild_id), COMMAND_AFK_ADDED));
}
