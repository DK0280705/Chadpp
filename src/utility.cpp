#include "utility.h"
#include "bot.h"
#include "input.h"
#include "language.h"
#include "logger.h"
#include <dpp/fmt/format.h>
#include <random>

const char* bool_string(bool b) noexcept
{
    return b ? "Yes" : "Nope";
}

bool icon_empty(const dpp::utility::iconhash& ih) noexcept
{
    return (ih.first == 0) & (ih.second == 0);
}

std::string ivectostr(const std::vector<int>& vec) noexcept
{
    std::string str;
    for (const auto& i : vec) str += std::to_string(i);
    return str;
}

bool isdigit(const std::string& str) noexcept
{
    for (const auto& c : str)
        if (!isdigit(c)) return false;
    return true;
}

int random(int from, int to) noexcept
{
    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_int_distribution<> dist(from, to);
    return dist(rng);
}

struct RequestProxy
{
    short tries;
    Input input;
    req_cb_t cb;

    void operator()(const dpp::http_request_completion_t& res) const
    {
        switch (res.status) {
        case 404:
            input.edit_reply(_(input->lang_id, CMD_ERR_REQUEST_NOT_FOUND));
        case 301:
        case 302:
            request(res.headers.at("location"), dpp::m_get, input, cb, tries + 1);
        case 200:
            cb(input, res);
        default:
            input.edit_reply(_(input->lang_id, CMD_ERR_CONN_FAILURE));
            log_err("REQUEST_PROXY", "Err: " + std::to_string(res.status));
        }
    }
};

void request(const std::string& u,
             dpp::http_method m,
             const Input& input,
             const req_cb_t cb,
             short tries)
{
    if (!tries) input.defer();
    (tries < 10) ? Bot::instance().request(u, m, RequestProxy {tries, input, cb})
                 : throw std::runtime_error("Too much redirect");
}

constexpr const char* WEBHOOK_PAYLOAD = R"({{
    "content": "{}",
    "avatar_url": "{}",
    "username": "{}",
    "embeds": [],
    "files": []
}})";

static inline void
post_rest(Bot* bot, uint64_t whid, const std::string& token, const std::string& payload)
{
    bot->post_rest(
        API_PATH "/webhooks", std::to_string(whid), token, dpp::m_post, payload,
        [](json&, const dpp::http_request_completion_t&) {});
}

// manual implementation, using cluster::execute_webhook is a pain.
void send_webhook(uint64_t channel_id,
                  const std::string& name,
                  const std::string& avatar,
                  const std::string& message)
{
    Bot& bot            = Bot::instance();
    std::string payload = fmt::format(WEBHOOK_PAYLOAD, message, avatar, name);
    bot.get_channel_webhooks(channel_id, [&bot, channel_id, payload = std::move(payload)]
                                         (const dpp::confirmation_callback_t& res) {
        const dpp::webhook_map& wm = std::get<dpp::webhook_map>(res.value);

        const auto wm_it = std::find_if(wm.begin(), wm.end(),
                                        [](const auto& wh) { return wh.second.name == "chadpp"; });
        if (wm_it != wm.end()) {
            post_rest(&bot, wm_it->second.id, wm_it->second.token, payload);
        } else {
            dpp::webhook w;
            w.name       = "chadpp";
            w.type       = dpp::w_incoming;
            w.channel_id = channel_id;
            bot.create_webhook(w, [&bot, payload = std::move(payload)](
                                      const dpp::confirmation_callback_t& res) {
                const dpp::webhook& wh = std::get<dpp::webhook>(res.value);
                post_rest(&bot, wh.id, wh.token, payload);
            });
        }
    });
}
