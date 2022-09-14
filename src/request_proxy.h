#pragma once
#include <dpp/queues.h>
#include <string>

typedef void (*req_cb_t)(const class Input&, const dpp::http_request_completion_t&);

void request(const std::string& url,
             dpp::http_method m,
             const class Input& i,
             const req_cb_t cb,
             short int t = 0);
