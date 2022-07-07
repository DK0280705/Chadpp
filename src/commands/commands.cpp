#include "commands.h"
#include "../module.h"

Bot* bot = nullptr;

extern "C" void destroy(Bot* bot, void*)
{
    std::unique_lock lock(bot->cmd_mutex); 
    for (auto& cmd : bot->commands) delete cmd.second;
    bot->commands.clear();
}

extern "C" Module* init(Bot* bot)
{
    Module* cmd_mod = (Module*)malloc(sizeof(Module));
    *cmd_mod        = {"commands", NULL, NULL};
    
    ::bot = bot;
    
    {
        std::unique_lock lock(bot->cmd_mutex);
        bot->commands = {
            {"afk",        new Command_afk()       },
            {"avatar",     new Command_avatar()    },
            {"bnc",        new Command_bnc()       },
            {"guildstats", new Command_guildstats()},
            {"help",       new Command_help()      },
            {"lang",       new Command_lang()      },
            {"ping",       new Command_ping()      },
            {"publish",    new Command_publish()   },
            {"purge",      new Command_purge()     },
            {"record",     new Command_record()    },
            {"reddit",     new Command_reddit()    },
            {"reload",     new Command_reload()    },
            {"sudo",       new Command_sudo()      },
            {"test",       new Command_test()      },
            {"urban",      new Command_urban()     }
        };
    }

    return cmd_mod;
}
