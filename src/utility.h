#pragma once
#include <dpp/queues.h>
#include <string>

namespace dpp {
namespace utility {
struct iconhash;
}
}

[[nodiscard]] const char* bool_string(bool b) noexcept;

[[nodiscard]] bool icon_empty(const dpp::utility::iconhash& ih) noexcept;

[[nodiscard]] std::string ivectostr(const std::vector<int>& vec) noexcept;

[[nodiscard]] bool isdigit(const std::string& str) noexcept;

[[nodiscard]] int random(int from, int to) noexcept;

typedef void (*req_cb_t)(const class Input&, const dpp::http_request_completion_t&);
void request(const std::string& u,
             dpp::http_method m,
             const class Input& i,
             const req_cb_t cb,
             short tries = 0);

void send_webhook(uint64_t channel_id,
                  const std::string& name,
                  const std::string& avatar,
                  const std::string& message);
