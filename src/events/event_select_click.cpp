#include "../application.h"
#include "../input.h"
#include "events.h"

void event_select_click(const dpp::select_click_t& event)
{
    std::shared_lock lock(bot->app_mutex);
    const auto app_it = bot->applications.find(event.custom_id);
    if (app_it != bot->applications.end()) {
        Command_options args(event.values.size());
        for (int i = 0; i < event.values.size(); i++)
            args[i] = event.values[i];

        Input input(bot, event.from, args, event.command);
        app_it->second->call(input);
    }
}
