#include "../application.h"
#include "../input.h"
#include "events.h"

void event_button_click(const dpp::button_click_t& event)
{
    const dpp::component_interaction& com_itr = std::get<dpp::component_interaction>(
        event.command.data);

    const std::string app_name = com_itr.custom_id.substr(0, com_itr.custom_id.find('_'));

    // Getting dummy id is impossible because i set the button to disabled for dummy buttons.
    // This is just verbose checking.
    if (app_name == "dummy") return;

    std::shared_lock lock(bot->app_mutex);
    const auto app_it = bot->applications.find(app_name);
    if (app_it != bot->applications.end()) {
        const std::string value    = com_itr.custom_id.substr(app_name.size() + 1);
        std::string::size_type pos = value.find('_', 0);
        Command_options args       = {
            {0, value.substr(0, pos)},
            {1, value.substr(pos + 1)}
        };

        Input input(bot, event.from, args, event.command);
        app_it->second->call(input);
    }
}