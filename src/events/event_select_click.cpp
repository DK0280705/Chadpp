#include "../application.h"
#include "../input.h"
#include "events.h"

void event_select_click(const dpp::select_click_t& event)
{
    std::shared_lock lock(bot->app_mutex);
    const auto app_it = bot->applications.find(event.custom_id);
    if (app_it != bot->applications.end()) {
        Command_options args;
        for (auto& val : event.values) args.emplace(args.size(), val);

        Input input(bot, event.from, args, event.command);
        app_it->second->call(input);
    }
}