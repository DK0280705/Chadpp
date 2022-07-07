#pragma once
#include <string>

extern "C" typedef struct Module
{
    const char* name;
    void* data;
    void* handle;
} Module;

bool load_module(class Bot* bot, const std::string& name) noexcept;

bool reload_module(class Bot* bot, const char* name) noexcept;

bool unload_module(class Bot* bot, Module* mod) noexcept;