#include "language.h"
#include <unordered_map>
#include <vector>

// I just don't want to pass bot pointer to gettext function
// Or event making gettext method on bot class
extern std::unordered_map<int, std::vector<const char*>> lang;

const char* _(int l_id, int text_id)
{
    return lang.at(l_id)[text_id];
}