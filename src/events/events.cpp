#include "events.h"
#include "../bot.h"

extern "C" typedef struct Handlers
{
    size_t btc;
    size_t sec;
    size_t slc;
    size_t log;
    size_t mcr;
    size_t mcl;
    size_t rdy;
    size_t vor;
} Handlers;

Bot* bot = nullptr;

extern "C" void* init(Bot* bot)
{
    Handlers* h = (Handlers*)malloc(sizeof(Handlers));

    ::bot = bot;

    h->btc = bot->on_button_click(&event_button_click);
    h->sec = bot->on_select_click(&event_select_click);
    h->slc = bot->on_slashcommand(&event_slashcommand);
    h->log = bot->on_log(&event_log);
    h->mcr = bot->on_message_create(&event_message_create);
    h->mcl = bot->on_message_create(&event_message_collect);
    h->rdy = bot->on_ready(&event_ready);
    h->vor = bot->on_voice_receive(&event_voice_receive);

    return h;
}

extern "C" void destroy(Bot* bot, void* data)
{
    Handlers* h = (Handlers*)data;
    bot->on_button_click.detach(h->btc);
    bot->on_select_click.detach(h->sec);
    bot->on_slashcommand.detach(h->slc);
    bot->on_log.detach(h->log);
    bot->on_message_create.detach(h->mcr);
    bot->on_message_create.detach(h->mcl);
    bot->on_ready.detach(h->rdy);
    bot->on_voice_receive.detach(h->vor);
    free(h);
}
