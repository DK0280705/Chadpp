#include "../collector.h"
#include "events.h"

void event_message_collect(const dpp::message_create_t& event)
{
    if (event.msg.author.is_bot()) return;
    std::shared_lock lock(bot->message_collector_mutex);
    if (!bot->message_collectors.empty()) {
        const auto mc_it = bot->message_collectors.find(event.msg.channel_id);
        if (mc_it != bot->message_collectors.end()) {
            lock.unlock();
            mc_it->second->execute(event.msg);
        }
    }
}