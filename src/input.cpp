#include "input.h"
#include "bot.h"

Input::Input(Bot* bot,
             dpp::discord_client* client,
             Command_options& args,
             const dpp::message& msg) noexcept
    : _bot(bot), _data(new Input_data)
{
    *_data = {std::move(args), msg.channel_id, msg.guild_id, msg.id,
              msg.author,      msg.member,     client,       bot->guild_lang(msg.guild_id), false};
}

Input::Input(Bot* bot,
             dpp::discord_client* client,
             Command_options& args,
             const dpp::interaction& itr) noexcept
    : _bot(bot), _data(new Input_data)
{
    *_data = {std::move(args), itr.channel_id,      itr.guild_id, itr.message_id,
              itr.usr,         itr.member,          client,       bot->guild_lang(itr.guild_id),
              false,           itr.id,              std::move(itr.token)};
}

Input::Input(const Input& i) noexcept
    : _bot(i._bot), _data(i._data)
{
    std::lock_guard lock(_mutex);
    (_data->_ref_count)++;
}

void Input::defer() const
{
    if (_data->is_itr)
        _bot->interaction_response_create(
            _data->id, _data->token,
            dpp::interaction_response(dpp::ir_deferred_channel_message_with_source,
                                      dpp::message()));
    else _bot->channel_typing(_data->channel_id);
}

void Input::reply(const std::string& content,
                  const dpp::interaction_response_type t,
                  const dpp::command_completion_event_t& cb) const
{
    reply(dpp::message(_data->channel_id, content), t, cb);
}

void Input::reply(const dpp::embed& content,
                  const dpp::interaction_response_type t,
                  const dpp::command_completion_event_t& cb) const
{
    reply(dpp::message(_data->channel_id, content), t, cb);
}

void Input::reply(const dpp::message& m,
                  const dpp::interaction_response_type t,
                  const dpp::command_completion_event_t& cb) const
{
    if (_data->is_itr)
        _bot->interaction_response_create(_data->id, _data->token, dpp::interaction_response(t, m),
                                          cb);
    else _bot->message_create(m, cb);
}

void Input::edit_reply(const std::string& content, const dpp::command_completion_event_t& cb) const
{
    dpp::message m(_data->channel_id, content);
    edit_reply(m, cb);
}

void Input::edit_reply(const dpp::embed& content, const dpp::command_completion_event_t& cb) const
{
    dpp::message m(_data->channel_id, content);
    edit_reply(m, cb);
}

void Input::edit_reply(dpp::message& m, const dpp::command_completion_event_t& cb) const
{
    if (_data->is_itr) _bot->interaction_response_edit(_data->token, m, cb);
    else if (_data->id == 0) _bot->message_create(m, cb);
    else {
        m.id = _data->id;
        _bot->message_edit(m);
    }
}

void Input::reply_sync(const std::string& content, const dpp::interaction_response_type t) const
{
    reply_sync(dpp::message(_data->channel_id, content), t);
}

void Input::reply_sync(const dpp::embed& embed, const dpp::interaction_response_type t) const
{
    reply_sync(dpp::message(_data->channel_id, embed), t);
}

void Input::reply_sync(const dpp::message& m, const dpp::interaction_response_type t) const
{
    if (_data->is_itr)
        _bot->interaction_response_create_sync(_data->id, _data->token,
                                               dpp::interaction_response(t, m));
    else _data->id = _bot->message_create_sync(m).id;
}

void Input::edit_reply_sync(const std::string& content) const
{
    dpp::message m(_data->channel_id, content);
    edit_reply_sync(m);
}

void Input::edit_reply_sync(const dpp::embed& embed) const
{
    dpp::message m(_data->channel_id, embed);
    edit_reply_sync(m);
}

void Input::edit_reply_sync(dpp::message& m) const
{
    if (_data->is_itr) _bot->interaction_response_edit_sync(_data->token, m);
    else if (_data->id == 0) _data->id = _bot->message_create_sync(m).id;
    else {
        m.id = _data->id;
        _bot->message_edit_sync(m);
    }
}

Input::~Input()
{
    if ((_data->_ref_count) == 0) {
        delete _data;
    } else {
        std::lock_guard lock(_mutex);
        (_data->_ref_count)--;
    }
}