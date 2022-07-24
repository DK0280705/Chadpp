#pragma once
#include <functional>
#include <pqxx/pqxx>
#include <string>


typedef std::function<void(const pqxx::result&)> Query_completion_callback;

class Database
{
public:
    Database(pqxx::connection* c);
    Database() = delete;
    Database(const Database&) = delete;
    ~Database();

    pqxx::connection* conn;

    bool terminating;

    void execute(const std::string& query, const Query_completion_callback& callback = {});

    pqxx::result execute_sync(const std::string& query);

private:
    void _worker();
};
