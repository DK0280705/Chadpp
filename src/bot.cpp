#include "bot.h"
#include "collector.h"
#include "command.h"
#include "database.h"
#include "input.h"
#include "language.h"
#include "logger.h"
#include "module.h"
#include <dlfcn.h>

#define DECLARE_COLLECTOR_HELPER_DEFINITION(__coll_name__, __coll_type__)                          \
    void Bot::add_##__coll_name__(dpp::snowflake id, __coll_type__* coll)                          \
    {                                                                                              \
        std::unique_lock lock(__coll_name__##_mutex);                                              \
        __coll_name__##s.emplace(id, coll);                                                        \
    }                                                                                              \
    void Bot::remove_##__coll_name__(dpp::snowflake id)                                            \
    {                                                                                              \
        std::unique_lock lock(__coll_name__##_mutex);                                              \
        delete __coll_name__##s.at(id);                                                            \
        __coll_name__##s.erase(id);                                                                \
    }

constexpr const int CMD_THREADS = 4;

constexpr const uint32_t INTENTS = dpp::i_guilds
    | dpp::i_message_content
    | dpp::i_guild_members
    | dpp::i_guild_messages
    | dpp::i_guild_voice_states;

extern char* DISCORD_TOKEN;

struct Input_request
{
    Command* cmd;
    Input input;
};

bool Bot::terminating = false;
std::unordered_map<int, std::vector<const char*>> lang;

static std::queue<struct Input_request> cmd_queue;
static std::condition_variable cmd_cv;
static std::vector<std::thread> cmd_threads;
static std::mutex worker_mutex;

static void _load_modules(Bot* bot)
{
    if (!load_module(bot, "applications") || !load_module(bot, "commands") ||
        !load_module(bot, "events") || !load_module(bot, "languages"))
        // Immediately terminates the program
        // as it violates the noexcept specifier
        // This is an evil code
        throw std::runtime_error(dlerror());
}

static void fill_cmd_options(std::vector<dpp::command_option>& options, int id)
{
    for (auto& opt : options) {
        id++;
        opt.description = _(0, id);
        if (opt.type < 3) fill_cmd_options(opt.options, id);
    }
}

Bot::Bot() noexcept
    : dpp::cluster(DISCORD_TOKEN, INTENTS)
    , loaded(false)
    , _start_time(dpp::utility::time_f())
{
    _load_modules(this);
    for (unsigned short i = 0; i < CMD_THREADS; i++)
        cmd_threads.emplace_back(std::thread(&Bot::_cmd_worker, this));
    log_info("BOT", "Starting...");
}

Bot& Bot::instance()
{
    static Bot bot;
    return bot;
}

std::vector<dpp::slashcommand> Bot::build_slashcommands() const
{
    std::vector<dpp::slashcommand> s_cmds;

    for (const auto&[name, c_ptr] : commands) {
        if (c_ptr->owner_only) continue;
        dpp::slashcommand cmd = dpp::slashcommand(name, _(0, c_ptr->desc_id), me.id);
        
        cmd.options = c_ptr->options;
        fill_cmd_options(cmd.options, c_ptr->desc_id);
        
        s_cmds.emplace_back(cmd);
    }

    return s_cmds;
}

void Bot::execute_cmd(Command* cmd, const Input& input)
{
    {
        std::lock_guard<std::mutex> lock(worker_mutex);
        cmd_queue.emplace(cmd, input);
    }
    cmd_cv.notify_one();
}

Command* Bot::find_command(const std::string& cmd_name) const
{
    const auto cmd_it = commands.find(cmd_name);
    if (cmd_it != commands.end()) return cmd_it->second;
    else
        for (const auto& c : commands)
            if (!c.second->alias.empty() && c.second->alias == cmd_name)
                return c.second;
    return nullptr;
}

// Unnecessary as it is
std::unordered_map<int, std::vector<const char*>>& Bot::lang() const
{
    return ::lang;
}

// Uhh, should i explain here?
// g_lang does not need mutex as if it modified, cmd_mutex handles it.
// only commands that uses g_lang, so it will be obvious
int Bot::guild_lang(dpp::snowflake id) const
{
    std::shared_lock lock(lang_mutex);
    if (!guild_lang_list.contains(id)) return 0;
    else return guild_lang_list.at(id);
}

int Bot::handle_command(Command* c, const Input& input) const
{
    if (c->owner_only && input->author.id != owner_id) return CMD_ERR_OWNER_ONLY;
    
    if (c->nsfw_only) {
        const dpp::channel* ch = dpp::find_channel(input->channel_id);
        if(!ch->is_nsfw()) return CMD_ERR_NSFW_ONLY; 
    }

    if (c->creates_m_coll && message_collectors.contains(input->channel_id))
        return CMD_ERR_MESSAGE_COLLECTOR_REJECT;

    if (c->permissions) {
        const dpp::channel* ch = dpp::find_channel(input->channel_id);
        const uint64_t perms   = ch->get_user_permissions(&input->author);
        if(!(perms & c->permissions)) return CMD_ERR_NEED_PERMISSION;
    }

    return NOTHING;
}

DECLARE_COLLECTOR_HELPER_DEFINITION(dummy_collector, Dummy_collector)
DECLARE_COLLECTOR_HELPER_DEFINITION(message_collector, Message_collector)

void Bot::_setup()
{
    if (loaded) return;
    log_info("BOT", "Setting up"); 

    const pqxx::result gs = database->execute_sync("SELECT * FROM chadpp.guild_spam");
    for (const auto& row : gs) guild_spam_list.emplace(row[0].as<uint64_t>(), true);

    const pqxx::result gl = database->execute_sync("SELECT * FROM chadpp.guild_lang");
    for (const auto& row : gl) guild_lang_list.emplace(row[0].as<uint64_t>(), row[1].as<int>()); 

    this->guild_bulk_command_create(build_slashcommands(), test_guild_id);
    loaded = true;
}

void Bot::_cmd_worker()
{
    std::unique_lock<std::mutex> lock(worker_mutex);
    while (true) {
        cmd_cv.wait(lock, []() { return !cmd_queue.empty() || Bot::terminating; });
        if (Bot::terminating) break;

        const Input_request ir = cmd_queue.front();
        cmd_queue.pop();

        lock.unlock();
        try {
            ir.cmd->call(ir.input);
        } catch (const std::exception& e) {
            log_err("CMD_WORKER", e.what());
        }
        lock.lock();
    }
}

Bot::~Bot()
{
    cmd_cv.notify_all();

    for (auto& cth : cmd_threads) cth.join();
    
    for (auto& dmc : dummy_collectors) dmc.second->destroy();

    for (auto& mcl : message_collectors) mcl.second->destroy();

    for (auto& mod : modules) unload_module(this, mod.second);
}
