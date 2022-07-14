#include "../database.h"
#include "commands.h"
#include <dpp/fmt/format.h>

Command_lang::Command_lang()
    : Command("lang", COMMAND_LANG_DESC,
        { dpp::command_option(dpp::co_integer, "name", "", true)
            .add_choice(dpp::command_option_choice("en", 0))
            .add_choice(dpp::command_option_choice("id", 1))})
{
    category = cat_general;
    permissions = dpp::p_manage_messages;
}

void Command_lang::call(const Input &input) const
{
    const int lang = std::get<int64_t>(input[0]);
    {
        std::unique_lock lock(bot->lang_mutex);
        bot->guild_lang_list.emplace(input->guild_id, lang);
    }
    bot->database.execute(fmt::format("EXECUTE upsert_guild_lang({}, {})", input->guild_id, lang));
    input.reply(_(lang, COMMAND_LANG_UPDATED));
}