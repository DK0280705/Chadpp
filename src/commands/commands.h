#pragma once
#include "../bot.h"
#include "../command.h"
#include "../input.h"

extern Bot* bot;

/**
 * @brief Inherited from popular c++ discord bot source code named TriviaBot
 * with some modifications
 */
#define DECLARE_COMMAND_CLASS(__command_name__)                                                    \
    class Command_##__command_name__ : public Command                                              \
    {                                                                                              \
    public:                                                                                        \
        Command_##__command_name__();                                                              \
        void call(const class Input& input) const override;                                        \
    };

DECLARE_COMMAND_CLASS(afk)
DECLARE_COMMAND_CLASS(avatar)
DECLARE_COMMAND_CLASS(bnc)
DECLARE_COMMAND_CLASS(guildstats)
DECLARE_COMMAND_CLASS(help)
DECLARE_COMMAND_CLASS(lang)
DECLARE_COMMAND_CLASS(ping)
DECLARE_COMMAND_CLASS(publish)
DECLARE_COMMAND_CLASS(purge)
DECLARE_COMMAND_CLASS(record)
DECLARE_COMMAND_CLASS(reddit)
DECLARE_COMMAND_CLASS(reload)
DECLARE_COMMAND_CLASS(sudo)
DECLARE_COMMAND_CLASS(test)
DECLARE_COMMAND_CLASS(urban)
