#include "applications.h"
#include "../module.h"
#include <mutex>

Bot* bot = nullptr;

extern "C" void destroy(Bot* bot, void*)
{
    std::unique_lock lock(bot->app_mutex);
    for (auto& app : bot->applications) delete app.second;
    bot->applications.clear();
}

extern "C" Module* init(Bot* bot)
{
    Module* app_mod = (Module*)malloc(sizeof(Module));
    *app_mod        = {"applications", NULL, NULL};

    ::bot = bot;

    {
        std::unique_lock lock(bot->app_mutex);
        bot->applications = {
            {"help",    new Application_help()   }
        };
    }

    return app_mod;
}