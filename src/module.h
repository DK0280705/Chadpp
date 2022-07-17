#pragma once

struct Module
{
    const char* name;
    void* data;
    void* handle;
};

bool load_module(class Bot* bot, const char* name) noexcept;

bool reload_module(class Bot* bot, const char* name) noexcept;

bool unload_module(class Bot* bot, Module& mod) noexcept;