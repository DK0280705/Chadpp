#include "../collector.h"
#include "../utility.h"
#include "commands.h"
#include <algorithm>
#include <dpp/fmt/format.h>

class Bulls_and_cows : public Collector<dpp::message>
{
public:
    Bulls_and_cows(Bot* bot, const Input& input, int length, int tries)
        : Collector(bot, 60, false, true)
        , _input(input)
        , _length(length)
        , _max_tries(tries)
        , _secret(_length, 0)
    {
        index = 1;
        generate_numbers();
        _e = dpp::embed()
            .set_title("Bulls and cows")
            .set_description(fmt::format("Guess a {} digit number", _length))
            .set_color(c_gray)
            .set_author({_input->author.username, {}, _input->author.get_avatar_url(), {}});
        _input.reply_sync(_e);
        _e.add_field("Attempts", {}, false);
    }

    void generate_numbers()
    {
        bool arr[10] = {false};
        for (size_t i = 0; i < _length; i++) {
            int num = random(0, 9);

            // Repeat if duplicate numbers
            if (arr[num]) i--;
            else {
                arr[num]   = true;
                _secret[i] = num;
            }
        }
    }

    bool on_collect(const dpp::message& item) override
    {
        if (item.content == "stop") {
            stop();
            return false;
        } else if (item.content.size() != _length || !isdigit(item.content)) return false;
        bot_->message_delete(item.id, _input->channel_id);
        
        _tries++;

        const std::string& ans = item.content;

        size_t bulls = 0;
        size_t cows  = 0;

        for (size_t i = 0; i < _length; i++)
            if ((ans[i] - 48) == _secret[i]) bulls++;
            else if (std::find(_secret.begin(), _secret.end(), (ans[i] - 48)) != _secret.end())
                cows++;

        const bool win = bulls == _secret.size();
        _e.fields[0].value += fmt::format("{}{} - {} Bulls, {} Cows - <@!{}>",
                                          _e.fields[0].value.empty() ? "" : "\n",
                                          win ? "**" + ans + "**" : "`" + ans + "`", bulls, cows,
                                          item.author.id);

        if (_answers.find(item.author.id) == _answers.end())
            _answers.emplace(item.author.id, 1);
        else _answers.at(item.author.id)++;

        _input.edit_reply(_e);

        if (win) {
            dpp::embed e = dpp::embed()
                .set_color(c_green)
                .set_description(fmt::format("<@!{}>, {}", item.author.id, _(_input->gl, COMMAND_BNC_GAME_WIN)))
                .set_footer({item.author.username, item.author.get_avatar_url(), {}});
            if (_answers.size() > 1) {
                uint64_t tryharder = 0;
                int most_val       = 0;
                for (const auto& [id, v] : _answers) {
                    int eval  = -(v > most_val);
                    most_val  = eval & v;
                    tryharder = eval & id;
                }
                e.add_field(_(_input->gl, COMMAND_BNC_GAME_TRYHARDER),
                            fmt::format("> <@!{}> - {} {}", tryharder, most_val, _(_input->gl, COMMAND_BNC_GAME_ATTEMPTS)), false);
            }
            bot_->message_create(dpp::message(_input->channel_id, e));
            stop();
            return false;
        } else if (_tries == _max_tries) {
            dpp::message m = dpp::message()
                .set_reference(_input->message_id)
                .add_embed(dpp::embed().set_title(_(_input->gl, COMMAND_BNC_GAME_MAX_TRIES_REACHED)));
            m.channel_id = _input->channel_id;
            bot_->message_create(m);
            stop();
            return false;
        }
        return true;
    }

    void on_end(const std::vector<dpp::message>&) override
    {
        _e.set_description(fmt::format("{} `{}`",
                           _(_input->gl, COMMAND_BNC_GAME_ENDED), ivectostr(_secret)));
        if (_e.fields[0].value.empty()) _e.fields.clear();
        _input.edit_reply(_e);
        bot_->remove_message_collector(_input->channel_id);
    }

private:
    Input _input;
    size_t _length;
    int _tries = 0;
    int _max_tries;

    std::vector<int> _secret;
    std::unordered_map<uint64_t, int> _answers;
    dpp::embed _e;
};

Command_bnc::Command_bnc()
    : Command("bnc",
              COMMAND_BNC_DESC,
              {{dpp::command_option(dpp::co_integer, "length", "")},
               {dpp::command_option(dpp::co_integer, "tries", "")}})
{
    creates_m_coll = true;
    category       = cat_fun;
}

void Command_bnc::call(const Input& input) const
{
    const int length = input.has(0) ? std::get<int64_t>(input[0]) : 4;
    const int tries  = input.has(1) ? std::get<int64_t>(input[1]) : 10;

    int succ = (length < 1 || tries < 1) ? COMMAND_BNC_INVALID_PARAMS
               : (length > 10)           ? COMMAND_BNC_INVALID_PARAM1
               : (tries > 20)            ? COMMAND_BNC_INVALID_PARAM2
                                         : NOTHING;

    if (succ != 0) return input.reply(_(input->gl, succ));

    bot->add_message_collector(input->channel_id,
                               new Bulls_and_cows(bot, input, length, tries));
}
