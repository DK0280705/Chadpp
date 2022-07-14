#include "../bot.h"
#include "../input.h"
#include "commands.h"
#include <dpp/fmt/format.h>
#include <dpp/version.h>

Command_help::Command_help()
    : Command("help", COMMAND_HELP_DESC,
              {{dpp::command_option(dpp::co_string, "name", "")}})
{
    category = cat_general;
}

static void cmd_opts_pretty(int guild_lang,
                            int desc_id,
                            const std::vector<dpp::command_option>& options,
                            std::string& description,
                            std::string& field)
{
    for (const auto& opt : options) {
        desc_id++;
        if (opt.type < 3) [[unlikely]] {
            const std::string gap = (opt.type == 1) ? "  " : "\t";
            description += "\n" + gap + "| " + opt.name;
            field += fmt::format("- **{}** : {}\n", opt.name, _(guild_lang, desc_id));
            cmd_opts_pretty(guild_lang, desc_id, opt.options, description, field);
        } else [[likely]] {
            description += ((opt.required) ? fmt::format(" <{}>", opt.name)
                                           : fmt::format(" [{}]", opt.name));
            field += fmt::format("> `{}` - {}\n", opt.name, _(guild_lang, desc_id));
            for (const auto& choice : opt.choices)
                field += fmt::format("::- {}", choice.name);
        }
    }
}

void Command_help::call(const Input& input) const
{
    if (input->args.empty()) {
        dpp::embed e = dpp::embed()
            .set_title(_(input->lang_id, COMMAND_HELP_EMBED_TITLE))
            .set_description(_(input->lang_id, COMMAND_HELP_EMBED_DESC))
            .set_color(c_gray)
            .set_thumbnail(bot->me.get_avatar_url())
            .set_footer({_(input->lang_id, COMMAND_HELP_EMBED_FOOTER),
                         bot->me.get_avatar_url(),
                         {}})
            .add_field(_(input->lang_id, INFORMATION), fmt::format("> Author: `DK_028`\n"
                                                       "> C++ version: `{}`\n"
                                                       "> D++ version: `{}`\n"
                                                       "> Shards: `{}`\n",
                                                       "CXX20", DPP_VERSION_TEXT,
                                                       bot->get_shards().size()));
        dpp::component c = dpp::component()
            .set_type(dpp::cot_selectmenu)
            .set_id("help")
            .set_min_values(1)
            .set_max_values(1)
            .set_placeholder(_(input->lang_id, COMMAND_HELP_COMP_SELECT_CAT))
            .add_select_option(
                dpp::select_option("General", "0", _(input->lang_id, COMMAND_HELP_COMP_CAT_GENERAL)))
            .add_select_option(
                dpp::select_option("Fun", "1", _(input->lang_id, COMMAND_HELP_COMP_CAT_FUN)))
            .add_select_option(
                dpp::select_option("Utility", "2", _(input->lang_id, COMMAND_HELP_COMP_CAT_UTILITY)))
            .add_select_option(
                dpp::select_option("Owner", "3", _(input->lang_id, COMMAND_HELP_COMP_CAT_OWNER)));

        dpp::message m = dpp::message(input->channel_id, e)
            .add_component(dpp::component()
                .add_component(c));

        input.reply(m);
    } else {
        const std::string& c_name = std::get<std::string>(input[0]);
        const Command* c          = bot->find_command(c_name);
        
        if (!c)
            return input.reply(dpp::embed().set_color(c_red).set_description(
                fmt::vformat(_(input->lang_id, COMMAND_HELP_CMD_NOT_FOUND), fmt::make_format_args(c_name))));

        std::string options_desc;
        std::string options_field;
        cmd_opts_pretty(input->lang_id, c->desc_id, c->options, options_desc, options_field);
        dpp::embed e = dpp::embed()
            .set_color(c_gray)
            .set_description(fmt::format("```{}{}{}```", bot->default_prefix,
                                         c->name, options_desc))
            .add_field(_(input->lang_id, COMMAND_HELP_CMD_DESC), _(input->lang_id, c->desc_id));
        
        if (!c->options.empty()) e.add_field(_(input->lang_id, COMMAND_HELP_CMD_PARAM), options_field);
        input.reply(e);
    }
}
