#include "applications.h"
#include <mutex>

Bot* bot = nullptr;

extern "C" void* init(Bot* bot)
{
    ::bot = bot;

    {
        std::unique_lock lock(bot->app_mutex);
        bot->applications = {
            {"help", new Application_help()}
        };
    }

    return NULL;
}

extern "C" void destroy(Bot* bot, void*)
{
    std::unique_lock lock(bot->app_mutex);
    for (auto& app : bot->applications) delete app.second;
    bot->applications.clear();
}