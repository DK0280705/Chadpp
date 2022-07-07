#include "bot.h"
#include "module.h"
#include <dlfcn.h>

bool load_module(Bot* bot, const std::string& name) noexcept
{
    const std::string filename = "./" + name + ".so"; 

    void* handle = dlopen(filename.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!handle) return false;

    typedef Module* (*init_t)(Bot*);
    init_t init = (init_t)dlsym(handle, "init");
    Module* mod = init(bot);
    mod->handle = handle;
    bot->modules[mod->name] = mod;

    return true;
}

bool reload_module(Bot* bot, const char* name) noexcept
{
    auto it = bot->modules.find(name);
    if (it != bot->modules.end()) {
        unload_module(bot, it->second);
        bot->modules.erase(name);
    }
    return load_module(bot, name);
}

bool unload_module(Bot* bot, Module* mod) noexcept
{
    typedef void (*destroy_t)(Bot*, void*);
    destroy_t destroy = (destroy_t)dlsym(mod->handle, "destroy");
    destroy(bot, mod->data);
    dlclose(mod->handle);
    free(mod);

    return true;
}
