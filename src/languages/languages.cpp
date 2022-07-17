#include "../bot.h"
#include <mutex>
#include <vector>

extern std::vector<const char*> langv_en;
extern std::vector<const char*> langv_id;
extern std::vector<const char*> langv_jp;

extern "C" void* init(Bot* bot)
{
    {
        std::unique_lock lock(bot->lang_mutex);
        bot->lang() = {
            {0, langv_en},
            {1, langv_id},
            {2, langv_jp}
        };
    }

    return NULL;
}

extern "C" void destroy(Bot* bot, void*)
{
    std::unique_lock lock(bot->lang_mutex);
    bot->lang().clear();
}