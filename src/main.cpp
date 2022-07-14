#include "bot.h"
#include "database.h"
#include "logger.h"
#include "module.h"
#include <dpp/nlohmann/json.hpp>

constexpr const char* EXIT_MESSAGE = "Terminating...";

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

static inline std::string jtypetostr(nlohmann::json::value_t t)
{
    static constexpr const char* jtypestr[] = {
        "null",    "object",           "array", "string", "boolean",
        "integer", "unsigned integer", "float", "binary", "discarded"};
    return jtypestr[static_cast<int>(t)];
}

int main()
{
    std::ifstream conf_file("config.json");
    if (!conf_file) {
        log_err("CONFIG", "Could NOT find file named 'config.json'");
        return 1;
    }

    nlohmann::json config;
    conf_file >> config;
    conf_file.close();

    constexpr const std::array<const char*, 5> keys = {
        "token", "botlog_id", "owner_id", "guild_id", "conn_string"
    };
    typedef nlohmann::json::value_t value_t;
    constexpr const std::array<value_t, 5> types = {
        value_t::string,
        value_t::number_unsigned,
        value_t::number_unsigned,
        value_t::number_unsigned,
        value_t::string
    };

    for (auto it = keys.begin(); it != keys.end(); it++) {
        auto c_it = std::find_if(config.items().begin(), config.items().end(),
                                 [&](const auto& i) { return i.key() == *it; });
        if(c_it == config.items().end()) {
            log_err("CONFIG", "Undefined config detected: " + std::string(*it));
            return 1;
        }
        if (config.at(*it).type() != types[it - keys.begin()]) {
            log_err("CONFIG", "Expected " + std::string(*it) + " to be a " +
                                  jtypetostr(types[it - keys.begin()]) +
                                  " instead got a value of " + jtypetostr(config.at(*it).type()) +
                                  " type");
            return 1;
        }
    }

    std::ifstream sql_file("chadpp.sql");
    if (!sql_file) {
        log_err("DATABASE", "chadpp.sql file not found");
        return 1;
    }

    try {
        pqxx::connection conn(config["conn_string"].get<std::string>());

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

        Database database(&conn);

        Bot& bot = Bot::init(config.at("token").get<std::string>(), database);

        bot.botlog_id      = config.at("botlog_id").get<uint64_t>();
        bot.owner_id       = config.at("owner_id").get<uint64_t>();
        bot.test_guild_id  = config.at("guild_id").get<uint64_t>();
        if (config.contains("prefix"))
            bot.default_prefix = config.at("prefix").get<std::string>();

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

        log_info("CACHE", "Total: " +
                              std::to_string(dpp::get_user_cache()->bytes() +
                                             dpp::get_role_cache()->bytes() +
                                             dpp::get_guild_cache()->bytes() +
                                             dpp::get_emoji_cache()->bytes() +
                                             dpp::get_channel_cache()->bytes()) +
                              "B");

    } catch (const std::exception& e) {
        log_err("BOT", e.what());
        return 1;
    }

    return 0;
}
