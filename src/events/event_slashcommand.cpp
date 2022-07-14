#include "../command.h"
#include "../database.h"
#include "../input.h"
#include "../variant_cast.h"
#include "events.h"

static int parse_interaction(const dpp::interaction& itr,
                             const std::vector<dpp::command_option>& c_options,
                             const std::vector<dpp::command_data_option>& itr_options,
                             Command_options& options)
{
    for (int i = 0, j = 0; i < itr_options.size(); i++, j++) {
        if (itr_options[i].name != c_options[j].name) {
            int size = c_options.size() - 1;
            do {
                j++;
                if (itr_options[i].name == c_options[j].name) break;
            } while (j < size);
        }

        const dpp::command_data_option& co = itr_options[i];
        dpp::command_parameter cp;

        switch (co.type) {
        case dpp::co_string:
        case dpp::co_integer:
        case dpp::co_boolean:
        case dpp::co_number:
        {
            cp = variant_cast(co.value);
            break;
        }
        case dpp::co_user:
        {
            const dpp::snowflake& uid = std::get<dpp::snowflake>(co.value);
            dpp::user* u              = dpp::find_user(uid);
            if (u) {
                dpp::resolved_user m;
                m.user        = *u;
                dpp::guild* g = dpp::find_guild(itr.guild_id);
                if (g->members.contains(uid)) m.member = g->members.at(uid);
                cp = m;
            } else return PARSE_ERR_USER;
            break;
        }
        case dpp::co_channel:
        {
            const dpp::snowflake& cid = std::get<dpp::snowflake>(co.value);
            dpp::channel* _c          = dpp::find_channel(cid);
            if (_c) cp = *_c;
            else return PARSE_ERR_CHANNEL;
            break;
        }
        case dpp::co_role:
        {
            const dpp::snowflake& rid = std::get<dpp::snowflake>(co.value);
            dpp::role* _r             = dpp::find_role(rid);
            if (_r) cp = *_r;
            else return PARSE_ERR_ROLE;
            break;
        }
        default:
        {
            options.emplace(options.size(), j);
            parse_interaction(itr, c_options[j].options, co.options, options);
            return NOTHING;
        }
        }
        options.emplace(j, cp);
    }
    return NOTHING;
}


void event_slashcommand(const dpp::slashcommand_t& event)
{
    bot->database.execute("EXECUTE find_afk_user(" + std::to_string(event.command.usr.id) + ")",
                            [usr_id = event.command.usr.id, ch_id = event.command.channel_id,
                             g_id = event.command.guild_id](const pqxx::result& res) {
        if (!res.empty()) {
            bot->database.execute_sync("DELETE FROM chadpp.afk_users WHERE id = " + std::to_string(usr_id));

            bot->message_create(dpp::message(ch_id, _(bot->guild_lang(g_id), COMMAND_AFK_REMOVED)));
        }
    });

    const dpp::command_interaction& cmd_itr = std::get<dpp::command_interaction>(event.command.data);


    std::shared_lock lock(bot->cmd_mutex);
    if (Command* c = bot->find_command(cmd_itr.name)) [[likely]] {
        Command_options options;
        
        int err = parse_interaction(event.command, c->options, cmd_itr.options, options);
        if (err == 0) {
            Input input(bot, event.from, options, event.command);
            int errh = bot->handle_command(c, input);
            if (errh == 0) bot->execute_cmd(c, input);
            else input.reply(_(input->lang_id, errh));
        } else event.reply(_(bot->guild_lang(event.command.guild_id), err));
    } else [[unlikely]] {
        lock.unlock();
        event.reply(dpp::message(0, _(bot->guild_lang(event.command.guild_id), CMD_ERR_REMOVED))
                        .set_flags(dpp::m_ephemeral));
    }
}
