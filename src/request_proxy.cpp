#include "bot.h"
#include "language.h"
#include "logger.h"
#include "request_proxy.h"
#include "input.h"

struct RequestProxy
{
    short tries;
    Input input;
    req_cb_t cb;

    void operator()(const dpp::http_request_completion_t& res) const
    {
        switch (res.status) {
        case 404:
            return input.edit_reply(_(input->lang_id, CMD_ERR_REQUEST_NOT_FOUND));
        case 301:
        case 302:
            return request(res.headers.at("location"), dpp::m_get, input, cb, tries + 1);
        case 200:
            return cb(input, res);
        default:
            input.edit_reply(_(input->lang_id, CMD_ERR_CONN_FAILURE));
            log_err("REQUEST_PROXY", "Err: " + std::to_string(res.status));
            return;
        }
    }
};

void request(const std::string& u,
             dpp::http_method m,
             const Input& i,
             const req_cb_t cb,
             short int t)
{
    if (!t) i.defer();
    (t < 10) ? Bot::instance().request(u, m, RequestProxy {t, i, cb})
                 : throw std::runtime_error("Too much redirect");
}
