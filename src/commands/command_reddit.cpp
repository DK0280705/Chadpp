#include "../utility.h"
#include "commands.h"
#include <dpp/fmt/format.h>
#include <dpp/nlohmann/json.hpp>

Command_reddit::Command_reddit()
    : Command("reddit", COMMAND_REDDIT_DESC,
              {{dpp::command_option(dpp::co_string, "r", "", true)}})
{
    alias    = "r";
    category = cat_util;
}

void Command_reddit::call(const Input& input) const
{
    const std::string& subreddit = std::get<std::string>(input[0]);

    request("https://www.reddit.com/r/" + subreddit + "/random.json", dpp::m_get, input,
            [](const Input& input, const dpp::http_request_completion_t& res) {
        json res_json;
        // Error killer
        try {
            res_json = json::parse(res.body);
        } catch (const nlohmann::detail::parse_error&) {}

        const json& children = res_json.is_array() ? res_json[0]["data"]["children"][0]
                                                   : res_json["data"]["children"][0];

        if (children.is_null())
            return input.edit_reply(_(input->lang_id, CMD_ERR_REQUEST_NOT_FOUND));

        const json& data = children["data"];
        dpp::embed e = dpp::embed()
            .set_title(data.at("title"))
            .set_color(c_orange)
            .set_url("https://www.reddit.com/" + data["permalink"].get<std::string>())
            .set_footer(
                {fmt::format("r/{} | Score : {} | rSubs : {}",
                                data["subreddit"].get<std::string>(),
                                data["score"].get<uint32_t>(),
                                data["subreddit_subscribers"].get<uint32_t>()),
                    {}, {}})
            .set_author(data.at("author"), "", "");
        if (data.contains("url")) e.set_image(data.at("url"));
        if (data.at("over_18")) {
            dpp::channel* c = dpp::find_channel(input->channel_id);
            if (!c->is_nsfw())
                return input.edit_reply(
                    _(input->lang_id, COMMAND_REDDIT_GOT_NSFW));
        }

        input.edit_reply(e);
    });
}
