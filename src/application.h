#pragma once
#include <string>

class Input;

class Application
{
public:
    Application(const char* name);
    virtual ~Application();
    virtual void call(const Input& input) const = 0;

    std::string name;

};
