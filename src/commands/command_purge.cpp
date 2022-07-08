#include "commands.h"

Command_purge::Command_purge()
    : Command("purge",
              COMMAND_PURGE_DESC,
              {{dpp::command_option(dpp::co_integer, "count", "", true)}})
{
    permissions = dpp::p_manage_messages;
    alias       = "prune";
}

void Command_purge::call(const Input& input) const
{
    short count = std::get<int64_t>(input[0]);

    int succ = (count > 100)  ? COMMAND_PURGE_INVALID_PARAM_MAX
               : (count == 0) ? COMMAND_PURGE_INVALID_PARAM_ZERO
               : (count == 1) ? COMMAND_PURGE_INVALID_PARAM_ONE
                              : NOTHING;

    if (succ != 0) return input.reply(_(input->gl, succ));

    if (input->id == 0) bot->message_delete(input->message_id, input->channel_id);
    else input.reply(dpp::message(0, "Done").set_flags(dpp::m_ephemeral));

    bot->messages_get(input->channel_id, 0, 0, 0, count,
                      [ch_id = input->channel_id](const dpp::confirmation_callback_t& cb) {
        std::vector<uint64_t> ids;
        for (const auto& msg : std::get<dpp::message_map>(cb.value)) ids.emplace_back(msg.first);
        bot->message_delete_bulk(ids, ch_id);
    });
}
