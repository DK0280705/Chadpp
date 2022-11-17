#include "../utility.h"
#include "commands.h"
#include <dpp/discordclient.h>
#include <dpp/fmt/format.h>

Command_sudo::Command_sudo()
    : Command("sudo",
              COMMAND_SUDO_DESC,
              {{dpp::command_option(dpp::co_user, "user", "", true)},
               {dpp::command_option(dpp::co_string, "text", "", true)}})
{
    category = cat_fun;
}

void Command_sudo::call(const Input& input) const
{
    if (!input->is_itr) bot->message_delete(input->message_id, input->channel_id);
    else input.reply(dpp::message(0, "Done!").set_flags(dpp::m_ephemeral));

    const dpp::resolved_user& u = std::get<dpp::resolved_user>(input[0]);

    const std::string avatar = isempty(u.member.avatar) ? u.user.get_avatar_url()
                                                           : u.member.get_avatar_url();

    const std::string name = u.member.nickname.empty() ? u.user.username : u.member.nickname;

    send_webhook(input->channel_id, name, avatar, std::get<std::string>(input[1]));
}
