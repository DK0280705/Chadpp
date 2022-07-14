#pragma once
#include <dpp/cluster.h>
#include <fstream>

#define DECLARE_COLLECTOR_HELPER(__coll_name__, __coll_type__)                                     \
    mutable std::shared_mutex __coll_name__##_mutex;                                                              \
    void add_##__coll_name__(dpp::snowflake id, __coll_type__* coll);                              \
    void remove_##__coll_name__(dpp::snowflake id);

class Application;
class Command;
class Database;
class Input;

template<class C> class Collector;
typedef Collector<dpp::message> Message_collector;
typedef Collector<struct None> Dummy_collector;

// chadpp predefined colors
constexpr uint32_t
    c_gray    = 0x2e3136,
    c_green   = 0x34a853,
    c_red     = 0xea4335,
    c_orange  = 0xff8a00,
    c_magenta = 0xd82560;

/**
 * @brief Bot class is a singleton.
 */
class Bot final : public dpp::cluster
{
public:
    Bot(const Bot&)            = delete;
    Bot(Bot&&)                 = delete;
    Bot& operator=(const Bot&) = delete;
    Bot& operator=(Bot&&)      = delete;
    ~Bot();

    static bool terminating;

    /**
     * @brief To get instance of Bot singleton.
     * Not for creating instance.
     */
    static Bot& instance();

    /**
     * @brief To create instance of Bot singleton.
     * @param token Discord Bot API Token
     * @param conn PostgreSQL libpqxx connection
     */
    static Bot& init(const std::string& token, Database& db) noexcept;

    bool loaded;
    Database& database;

    dpp::snowflake botlog_id     = 0;
    dpp::snowflake owner_id      = 0;
    dpp::snowflake test_guild_id = 0;
    std::string default_prefix   = "c!";
 
    std::unordered_map<std::string, Command*> commands;
    std::unordered_map<std::string, Application*> applications;

    // Spam check enabled guilds
    std::unordered_map<dpp::snowflake, bool> guild_spam_list;

    // Guild localization
    std::unordered_map<dpp::snowflake, int> guild_lang_list;

    // Voice recording streams
    // Channel id -> Audio streams
    std::unordered_map<dpp::snowflake, std::ofstream> stream_files;

    // Active dummy collectors
    // Dummy collectors are attached to user id
    // User id -> Dummy collector
    std::unordered_map<dpp::snowflake, Dummy_collector*> dummy_collectors;

    // Active message collectors
    // Channel id -> Message collector
    std::unordered_map<dpp::snowflake, Message_collector*> message_collectors;

    // Dynamic library handlers
    std::unordered_map<std::string, struct Module*> modules;

    // Why there's so many mutex here?
    // Multithreading kinda pain
    mutable std::shared_mutex app_mutex;
    mutable std::shared_mutex cmd_mutex;
    mutable std::shared_mutex lang_mutex;
    mutable std::shared_mutex voice_mutex;

    /**
     * @brief Creates vector of slashcommands from commands.
     */
    std::vector<dpp::slashcommand> build_slashcommands() const;

    /**
     * @brief Executes command to another thread.
     * @param cmd Command pointer.
     * @param input Input object.
     */
    void execute_cmd(Command* cmd, const Input& input);

    /**
     * @brief Find command with specified name or alias.
     * @param cmd_name Obviously command name or alias
     */
    Command* find_command(const std::string& cmd_name) const;

    /**
     * @brief Unnecessary complex function
     * Just to make sure i can fill the lang global var through bot pointer
     */
    std::unordered_map<int, std::vector<const char*>>& lang() const;

    /**
     * @brief Get guild language configuration
     * @param id guild id
     */
    int guild_lang(dpp::snowflake id) const; 
    
    /**
     * @brief Handle command, reject input that has invalid permissions
     * @param c Command pointer
     * @param input The input interface
     */
    int handle_command(Command* c, const Input& input) const;


    DECLARE_COLLECTOR_HELPER(dummy_collector, Dummy_collector)
    DECLARE_COLLECTOR_HELPER(message_collector, Message_collector)

    friend void event_ready(const dpp::ready_t& event);

private:
    Bot(const std::string& token, Database& db) noexcept;
    
    static Bot* _p_instance;

    double _start_time;

    void _setup();
    void _cmd_worker();
};
