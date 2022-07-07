#include "commands.h"

Command_publish::Command_publish()
    : Command("publish", COMMAND_PUBLISH_DESC, {})
{
    owner_only = true;
    category   = cat_owner;
}

void Command_publish::call(const Input& input) const
{
    bot->global_bulk_command_create_sync(bot->build_slashcommands());
    input.reply("Published");
}
