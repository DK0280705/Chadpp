#include "../utility.h"
#include "commands.h"
#include <dpp/fmt/format.h>
#include <dpp/nlohmann/json.hpp>

Command_urban::Command_urban()
    : Command("urban", COMMAND_URBAN_DESC,
              {{dpp::command_option(dpp::co_string, "query", "", true)}})
{
    category = cat_fun;
}

void Command_urban::call(const Input& input) const
{
    std::string query = std::get<std::string>(input[0]);
    std::replace(query.begin(), query.end(), ' ', '+');
    input.defer();
    bot->request("https://api.urbandictionary.com/v0/define?term=" + query, dpp::m_get,
                 [input](const dpp::http_request_completion_t& res) {
        if (res.status == 200) {
            const json data = json::parse(res.body);
            const json& obj = data.at("list").at(random(0, data.at("list").size() - 1)); 
            
            std::string def = obj.at("definition").get<std::string>(); 
            
            std::string ex = obj.at("example").get<std::string>();

            std::string word = obj.at("word").get<std::string>();
            word[0]          = toupper(word[0]);

            dpp::embed e = dpp::embed()
                .set_title(word)
                .set_color(c_gray)
                .set_url(obj["permalink"].get<std::string>())
                .set_description(def)
                .add_field("Example", ex)
                .set_footer({fmt::format("👍: {} | 👎: {}", obj.at("thumbs_up").get<int>(),
                                         obj.at("thumbs_down").get<int>()), {}, {}});
            input.edit_reply(e);
        } else if (res.status == 404)
            return input.edit_reply(_(bot->g_lang(input->guild_id), CMD_ERR_REQUEST_NOT_FOUND));
        else return input.edit_reply(_(bot->g_lang(input->guild_id), CMD_ERR_CONN_FAILURE));
    });
}