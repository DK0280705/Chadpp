#include "events.h"

void event_ready(const dpp::ready_t& event)
{
    bot->set_presence(dpp::presence(dpp::ps_online, dpp::at_listening, "Gachi Muchi"));
    if (event.shard_id == 0) {
        bot->_setup();

        const dpp::embed embed = dpp::embed()
            .set_title("Im ready mfs")
            .set_color(c_green);
        bot->message_create(dpp::message(bot->botlog_id, embed));
        bot->log(dpp::ll_debug,
                 "Start up time: " + std::to_string(dpp::utility::time_f() - bot->_start_time) + "s");
    }
}