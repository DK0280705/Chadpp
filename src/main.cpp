#include "bot.h"
#include "database.h"
#include "logger.h"
#include "module.h"
#include <stdexcept>

constexpr const char* EXIT_MESSAGE = "Terminating...";

char* DISCORD_TOKEN;

enum env_types
{
    string,
    integer
};

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

int main()
{
    std::unordered_map<const char*, std::variant<uint64_t, char*>> config(5);

    constexpr const std::array<const char*, 5> keys = {
        "DISCORD_TOKEN", "BOTLOG_ID", "OWNER_ID", "GUILD_ID", "CONN_STRING"
    };

    constexpr const env_types types[] = {
        string, integer, integer, integer, string
    };

    for (auto it = keys.begin(); it != keys.end(); it++) {
        char* env = std::getenv(*it);
        if (!env) {
            log_err("CONFIG", "Undefined config detected: " + std::string(*it));
            return 1;
        }
        if (types[it - keys.begin()]) {
            try {
                uint64_t e = std::stoull(env);
                config[*it] = e;
            } catch (const std::invalid_argument&) {
                log_err("CONFIG", "Expected integer value at key: " + std::string(*it));
                return 1;
            }
        } else config[*it] = env;
    }

    DISCORD_TOKEN = std::get<char*>(config.at("DISCORD_TOKEN"));

    std::ifstream sql_file("chadpp.sql");
    if (!sql_file) {
        log_err("DATABASE", "chadpp.sql file not found");
        return 1;
    }

    try {
        pqxx::connection conn(std::get<char*>(config.at("CONN_STRING")));

        if (conn.is_open()) {
            log_info("DATABASE", "Database started!\n  DBName\t: " + std::string(conn.dbname()) +
                                     "\n  PID\t\t: " + std::to_string(conn.backendpid()));

            sql_file.seekg(0, std::ios::end);
            const size_t size = sql_file.tellg();
            std::string sql(size, ' ');
            sql_file.seekg(0);
            sql_file.read(&sql[0], size);

            pqxx::work w(conn);
            w.exec(sql);
            w.commit();
        } else {
            log_err("DATABASE",
                    "Error on opening database, make sure PostgreSQL Server is running");
            return 1;
        }

        Database db(&conn);

        Bot& bot = Bot::instance();

        bot.database      = &db;
        bot.botlog_id     = std::get<uint64_t>(config.at("BOTLOG_ID"));
        bot.owner_id      = std::get<uint64_t>(config.at("OWNER_ID"));
        bot.test_guild_id = std::get<uint64_t>(config.at("GUILD_ID"));
        if (config.contains("PREFIX"))
            bot.default_prefix = std::get<uint64_t>(config.at("PREFIX"));

        bot.set_websocket_protocol(dpp::ws_etf);

        // Do nothing when Ctrl+C Called
        std::signal(SIGINT, [](int){});

        bot.start(true);

        std::string str;
        std::ios_base::sync_with_stdio(false);
        while (!Bot::terminating) {
            getline(std::cin, str);
            switch(str[0]) {
            case 'r': {
                std::string arg = str.substr(2);
                trim(arg);
                reload_module(&bot, arg.c_str());
                log_info("BOT", "Reloaded " + arg);
                break;
            }
            case 'q':
                Bot::terminating = true;
                log_info("BOT", EXIT_MESSAGE);
                break;
            default:
                log_err("MAIN", "Unknown terminal command");
                break;
            }
        }

        log_info("CACHE", "Total: "
            + std::to_string(
                dpp::get_user_cache()->bytes()
                    + dpp::get_role_cache()->bytes()
                    + dpp::get_guild_cache()->bytes()
                    + dpp::get_emoji_cache()->bytes()
                    + dpp::get_channel_cache()->bytes())
            + "B");

    } catch (const std::exception& e) {
        log_err("BOT", e.what());
        return 1;
    }

    return 0;
}
