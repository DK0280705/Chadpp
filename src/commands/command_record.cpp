#include "../logger.h"
#include "commands.h"
#include <filesystem>
extern "C" {
#include <lame/lame.h>
}

constexpr const int PCM_SIZE = 8192;
constexpr const int MP3_SIZE = 8192;

Command_record::Command_record()
    : Command("record",
              COMMAND_RECORD_DESC,
              {{dpp::command_option(dpp::co_sub_command, "start", "")},
               {dpp::command_option(dpp::co_sub_command, "stop", "")}})
{
    owner_only = true;
    category   = cat_util;
}

void Command_record::call(const Input& input) const
{
    // Why not using dpp::guild::connect_member_void()?
    // I want to get the voice channel id.
    int64_t sub = std::get<int64_t>(input[0]);

    int gl = input->gl;

    if (sub == 0) {
        if (input->client->get_voice(input->guild_id))
            return input.reply(_(gl, COMMAND_RECORD_IN_VC));

        dpp::guild* g = dpp::find_guild(input->guild_id);

        for (auto& ch_id : g->channels) {
            dpp::channel* ch = dpp::find_channel(ch_id);
            if (!ch || (!ch->is_voice_channel() && !ch->is_stage_channel())) continue;
            auto vc_members = ch->get_voice_members();

            auto vsi = vc_members.find(input->author.id);
            if (vsi != vc_members.end()) {
                input.reply(_(gl, COMMAND_RECORD_START));
                vsi->second.shard->connect_voice(input->guild_id, vsi->second.channel_id);

                std::unique_lock lock(bot->voice_mutex);
                bot->stream_files.emplace(vsi->second.channel_id,
                                          std::to_string(vsi->second.channel_id) + ".pcm");
                return;
            }
        }
        input.reply(_(gl, COMMAND_RECORD_USR_NOT_IN_VC));
    } else if (sub == 1) {
        if (dpp::voiceconn* vc = input->client->get_voice(input->guild_id)) {
            dpp::channel* ch = dpp::find_channel(vc->channel_id);

            auto vc_members = ch->get_voice_members();

            if (vc_members.find(input->author.id) == vc_members.end())
                return input.reply(_(gl, COMMAND_RECORD_USR_DIFF_VC));

            const dpp::snowflake ch_id = vc->channel_id;
            {
                std::unique_lock lock(bot->voice_mutex);
                bot->stream_files.at(ch_id).close();
                bot->stream_files.erase(ch_id);
            }

            input->client->disconnect_voice(input->guild_id);

            int16_t pcm_buffer[PCM_SIZE * 2];
            uint8_t mp3_buffer[MP3_SIZE];

            lame_t lame = lame_init();
            if (lame_init_params(lame) < 0)
                return input.reply(_(gl, COMMAND_RECORD_ERR_PROC_AUDIO));

            lame_set_in_samplerate(lame, 48000);
            lame_set_VBR(lame, vbr_default);
            lame_set_VBR_q(lame, 5);
            lame_set_num_channels(lame, 2);
            lame_set_mode(lame, STEREO);

            input.defer();

            const std::string pcm_f = "./" + std::to_string(ch_id) + ".pcm";
            const std::string mp3_f = "./" + std::to_string(ch_id) + ".mp3";
            std::ifstream pcm(pcm_f, std::ios::binary);
            std::ofstream mp3(mp3_f, std::ios::binary);

            while (pcm.good()) {
                int write = 0;
                pcm.read(reinterpret_cast<char*>(pcm_buffer), sizeof(pcm_buffer));
                int read = pcm.gcount() / 4; // 2*sizeof(int16_t)
                write    = (read == 0) ? lame_encode_flush(lame, mp3_buffer, MP3_SIZE)
                                       : lame_encode_buffer_interleaved(lame, pcm_buffer, read,
                                                                        mp3_buffer, MP3_SIZE);
                mp3.write(reinterpret_cast<char*>(mp3_buffer), write);
            }

            pcm.close();
            mp3.close();
            lame_close(lame);

            dpp::message m = dpp::message(input->channel_id, _(gl, COMMAND_RECORD_STOP))
                .add_file("recording.mp3", dpp::utility::read_file(mp3_f));
            input.edit_reply(m);

            if (std::filesystem::remove(pcm_f) && std::filesystem::remove(mp3_f))
                log_info("COMMAND:record", "Removed pcm and mp3 files");
        } else input.reply(_(gl, COMMAND_RECORD_DISCONNECTED));
    }
}
