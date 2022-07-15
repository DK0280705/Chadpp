#include "../command.h"
#include "applications.h"

Application_help::Application_help() : Application("help") {}

void Application_help::call(const Input& input) const
{
    const std::string& value   = std::get<std::string>(input[0]);
    const Command_category cat = static_cast<Command_category>(std::stoi(value));
    
    const char* title = [&]() {
        switch (cat) {
        case cat_general:
            return "General Commands";
        case cat_fun:
            return "Fun Commands";
        case cat_util:
            return "Utility Commands";
        case cat_owner:
            return "Owner Commands";
        }
    }();

    std::string cmd_list;
    {
        std::shared_lock lock(bot->cmd_mutex);
        for (const auto&[n, c] : bot->commands)
            if (c->category == cat)
                cmd_list += "> `" + bot->default_prefix + n + "` - " +
                            _(input->lang_id, c->desc_id) + "\n";
    }

    dpp::embed e = dpp::embed()
        .set_title(title)
        .set_color(c_gray)
        .set_thumbnail(bot->me.get_avatar_url())
        .set_footer({_(input->lang_id, COMMAND_HELP_EMBED_FOOTER),
                    bot->me.get_avatar_url(), {}});
    if (cmd_list.empty()) e.set_description(_(input->lang_id, APPLICATION_HELP_EMPTY_CAT));
    else e.add_field(_(input->lang_id, APPLICATION_HELP_CMD_LIST), cmd_list);

    const dpp::component c = dpp::component()
        .set_type(dpp::cot_selectmenu)
        .set_id("help")
        .set_min_values(1)
        .set_max_values(1)
        .set_placeholder(_(input->lang_id, COMMAND_HELP_COMP_SELECT_CAT))
        .add_select_option(dpp::select_option("General", "0", _(input->lang_id, COMMAND_HELP_COMP_CAT_GENERAL)))
        .add_select_option(dpp::select_option("Fun", "1", _(input->lang_id, COMMAND_HELP_COMP_CAT_FUN)))
        .add_select_option(dpp::select_option("Utility", "2", _(input->lang_id, COMMAND_HELP_COMP_CAT_UTILITY)))
        .add_select_option(dpp::select_option("Owner", "3", _(input->lang_id, COMMAND_HELP_COMP_CAT_OWNER)));


    input.reply(dpp::message(0, e).add_component(dpp::component().add_component(c)),
                dpp::ir_update_message);
}