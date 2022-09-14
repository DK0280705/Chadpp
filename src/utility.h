#pragma once
#include <dpp/queues.h>
#include <string>

namespace dpp {
namespace utility {
struct iconhash;
}
}

[[nodiscard]] const char* btostr(bool b) noexcept;

[[nodiscard]] bool isempty(const dpp::utility::iconhash& ih) noexcept;

[[nodiscard]] std::string ivectostr(const std::vector<int>& vec) noexcept;

[[nodiscard]] bool isdigit(const std::string& str) noexcept;

[[nodiscard]] int random(int from, int to) noexcept;

void send_webhook(uint64_t channel_id,
                  const std::string& name,
                  const std::string& avatar,
                  const std::string& message);
