#include "commands.h"

//include header to test here
//#include "../blablabla.h"

Command_test::Command_test()
    : Command("test", COMMAND_TEST_DESC, { { dpp::command_option(dpp::co_string, "test_param", "") } })
{
    owner_only = true;
    category   = cat_owner;
}

void Command_test::call(const Input& input) const
{
    input.reply_sync("This does nothing as you did not make any code here");
}