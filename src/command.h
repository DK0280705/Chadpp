#pragma once
#include <dpp/appcommand.h>

enum Command_category
{
    cat_general = 0,
    cat_fun     = 1,
    cat_util,
    cat_owner
};

class Bot;
class Input;

class Command
{
public:
    Command(const char* name,
            const int desc_id,
            const std::vector<dpp::command_option>& opts);
    virtual void call(const Input& input) const = 0;
    virtual ~Command();

    bool nsfw_only       = false;
    bool owner_only      = false;
    bool creates_m_coll  = false;
    uint64_t permissions = 0;

    const std::string name;
    const int desc_id;
    const std::vector<dpp::command_option> options;
    Command_category category = cat_general;
    std::string alias;

};
