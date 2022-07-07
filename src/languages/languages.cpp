#include "../bot.h"
#include "../module.h"
#include <mutex>
#include <vector>

extern std::vector<const char*> langv_en;
extern std::vector<const char*> langv_id;
extern std::vector<const char*> langv_jp;

extern "C" void destroy(Bot* bot, void*)
{
    std::unique_lock lock(bot->lang_mutex);
    bot->lang().clear();
}

extern "C" Module* init(Bot* bot)
{
    Module* lang_mod = (Module*)malloc(sizeof(Module));
    *lang_mod        = {"languages", NULL, NULL};
    
    {
        std::unique_lock lock(bot->lang_mutex);
        bot->lang() = {
            {0, langv_en},
            {1, langv_id},
            {2, langv_jp}
        };
    }

    return lang_mod;
}