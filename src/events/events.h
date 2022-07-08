#pragma once
#include "../bot.h"
#include "../language.h"

// WTF is this macro

#define DECLARE_EVENT_FUNC_(__func_name__, __type_name__)                                          \
    void event_##__func_name__(const dpp::__type_name__##_t& event);

#define DECLARE_EVENT_FUNC_1(__func_name__) DECLARE_EVENT_FUNC_(__func_name__, __func_name__)

#define DECLARE_EVENT_FUNC_2(__func_name__, __type_name__)                                         \
    DECLARE_EVENT_FUNC_(__func_name__, __type_name__)

#define GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define DECLARE_EVENT_FUNC_MACRO_CHOOSER(...)                                                      \
    GET_3RD_ARG(__VA_ARGS__, DECLARE_EVENT_FUNC_2, DECLARE_EVENT_FUNC_1, )

#define DECLARE_EVENT_FUNC(...) DECLARE_EVENT_FUNC_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

namespace dpp
{

struct button_click_t;
struct message_create_t;
struct log_t;
struct ready_t;
struct select_click_t;
struct slashcommand_t;
struct voice_receive_t;

} // namespace dpp

extern Bot* bot;

DECLARE_EVENT_FUNC(button_click)
DECLARE_EVENT_FUNC(message_create)
DECLARE_EVENT_FUNC(message_collect, message_create)
DECLARE_EVENT_FUNC(log)
DECLARE_EVENT_FUNC(ready)
DECLARE_EVENT_FUNC(select_click)
DECLARE_EVENT_FUNC(slashcommand)
DECLARE_EVENT_FUNC(voice_receive)