#include "bot.h"
#include "database.h"
#include "logger.h"
#include <exception>

constexpr const char* EXIT_MESSAGE = "Terminating...";

char* discord_token;
char* conn_string;

static char* env(const char* key)
{
    char* e = std::getenv(key);
    if (!e) throw std::range_error(key);
    return e;
}

static inline void ltrim(std::string &s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

static void init_sql(pqxx::connection& conn, std::ifstream& file)
{
    log_info("DATABASE", "Database started!\n  DBName\t: " + std::string(conn.dbname()) +
                                 "\n  PID\t\t: " + std::to_string(conn.backendpid()));
    file.seekg(0, std::ios::end);
    const size_t size = file.tellg();
    std::string sql(size, ' ');
    file.seekg(0);
    file.read(&sql[0], size);

    pqxx::work w(conn);
    w.exec(sql);
    w.commit();
}

int main()
{
    // Integer keys
    static const char* keys[] = { "BOTLOG_ID", "OWNER_ID", "GUILD_ID" };
    static uint64_t values[] = { 0, 0, 0 };

    try {
        discord_token = env("DISCORD_TOKEN");
        conn_string   = env("CONN_STRING");
        for (int i = 0; i < 3; i++) {
            const char* e = env(keys[i]);
            try {
                values[i] = std::stoull(e);
            } catch (const std::invalid_argument& e) {
                log_err("CONFIG", std::string("Expected integer value at key: ") + e.what()); 
                return 1;
            }
        }
    } catch (const std::range_error& e) {
        log_err("CONFIG", std::string("Missing config: ") + e.what());
        return 1;
    }

    Database db;

    std::ifstream file("chadpp.sql");
    if (!file) {
        log_err("DATABASE", "chadpp.sql file not found");
        return 1;
    }

    if (db.conn->is_open()) {
        init_sql(*db.conn, file);
    } else {
        log_err("DATABASE", "Error on opening database, make sure PostgreSQL Server is running");
        return 1;
    }

    try {
        Bot& bot = Bot::instance();
         
        bot.database      = &db;
        bot.botlog_id     = values[0];
        bot.owner_id      = values[1];
        bot.test_guild_id = values[2];

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
