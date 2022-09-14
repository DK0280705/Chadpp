#include "../utility.h"
#include "commands.h"

Command_avatar::Command_avatar()
    : Command("avatar", COMMAND_AVATAR_DESC, {{dpp::command_option(dpp::co_user, "user", "")}})
{
    alias    = "pp";
    category = cat_general;
}

void Command_avatar::call(const Input& input) const
{
    const std::string avatar_url = [&]() {
        if (input->args.empty())
            return isempty(input->member.avatar) ? input->author.get_avatar_url(1024)
                                                 : input->member.get_avatar_url(1024);
        else {
            const dpp::resolved_user& ru = std::get<dpp::resolved_user>(input[0]);
            return isempty(ru.member.avatar) ? ru.user.get_avatar_url(1024)
                                             : ru.member.get_avatar_url(1024);
        }
    }();

    const dpp::embed e = dpp::embed()
        .set_image(avatar_url)
        .set_footer({"Cool.", {}, {}})
        .set_color(c_gray);
    input.reply(e);
}
