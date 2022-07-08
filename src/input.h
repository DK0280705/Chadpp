#pragma once
#include <dpp/commandhandler.h>
#include <dpp/discordclient.h>
#include <dpp/dispatcher.h>

typedef std::unordered_map<int, dpp::command_parameter> Command_options;

class Bot;

struct Input_data
{
    Command_options args;

    dpp::snowflake channel_id;
    dpp::snowflake guild_id;
    dpp::snowflake message_id;

    dpp::user author;
    dpp::guild_member member;
    dpp::discord_client* client;

    // Guild language 
    int gl;

    // Can behave as message id or interaction id
    mutable dpp::snowflake id = 0;
    std::string token;
};

/**
 * Self implemented shared pointer
 */
class Input
{
public:
    Input(Bot* bot,
          dpp::discord_client* client,
          Command_options& args,
          const dpp::message& msg) noexcept;
    Input(Bot* bot,
          dpp::discord_client* client,
          Command_options& args,
          const dpp::interaction& itr) noexcept;
    Input(const Input& i) noexcept;
    ~Input();

    inline const dpp::command_parameter& operator[](const int i) const { return _data->args.at(i); }
    inline bool has(const int i) const { return _data->args.contains(i); }
    inline const Input_data* operator->() const { return _data; }

    /**
     * @brief Defer to an interaction or response it as typing.
     */
    void defer() const;

    /**
     * @brief Fast reply with async function
     */
    void reply(const std::string& content,
               const dpp::interaction_response_type t    = dpp::ir_channel_message_with_source,
               const dpp::command_completion_event_t& cb = {}) const;
    void reply(const dpp::embed& content,
               const dpp::interaction_response_type t    = dpp::ir_channel_message_with_source,
               const dpp::command_completion_event_t& cb = {}) const;
    void reply(const dpp::message& m,
               const dpp::interaction_response_type t    = dpp::ir_channel_message_with_source,
               const dpp::command_completion_event_t& cb = {}) const;
    void edit_reply(const std::string& content,
                    const dpp::command_completion_event_t& cb = {}) const;
    void edit_reply(const dpp::embed& content,
                    const dpp::command_completion_event_t& cb = {}) const;
    void edit_reply(dpp::message& m, const dpp::command_completion_event_t& cb = {}) const;

    /**
     * @brief Reply with this method if you want to configure your message yourself.
     * @param m Message to send
     * @param t Response type for interaction
     */
    void
    reply_sync(const dpp::message& m,
               const dpp::interaction_response_type t = dpp::ir_channel_message_with_source) const;

    /**
     * @brief Fast reply function with string content.
     * @param content String content
     * @param t Response type for interaction
     */
    void
    reply_sync(const std::string& content,
               const dpp::interaction_response_type t = dpp::ir_channel_message_with_source) const;

    /**
     * @brief Fast reply function with embed content.
     * @param embed Embed content
     * @param t Response type for interaction
     */
    void
    reply_sync(const dpp::embed& embed,
               const dpp::interaction_response_type t = dpp::ir_channel_message_with_source) const;

    /**
     * @brief Edit reply with this method if you want to configure message yourself.
     * Only use this in callback function for confirmation_callback_t.
     * @param event Callback type retrieved from reply callback
     * @param m Message to send
     */
    void edit_reply_sync(dpp::message& m) const;

    /**
     * @brief Fast edit reply method with string content.
     * Only use this in callback function for confirmation_callback_t
     * @param event Callback type retrieved from reply callback
     * @param content String content
     */
    void edit_reply_sync(const std::string& content) const;

    /**
     * @brief Fast edit reply method with string content.
     * Only use this in callback function for confirmation_callback_t
     * @param event Callback type retrieved from reply callback
     * @param embed Embed content
     */
    void edit_reply_sync(const dpp::embed& embed) const;

private:
    Bot* _bot;
    Input_data* _data;
    std::mutex _mutex;
    short int* _counter;
    bool _itr = false;
};
