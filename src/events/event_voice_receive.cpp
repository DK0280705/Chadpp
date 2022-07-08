#include "events.h"

void event_voice_receive(const dpp::voice_receive_t& event)
{
    std::unique_lock lock(bot->voice_mutex);
    bot->stream_files.at(event.voice_client->channel_id)
        .write(reinterpret_cast<char*>(event.audio), event.audio_size);
}