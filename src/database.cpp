#include "database.h"
#include "logger.h"
#include <condition_variable>
#include <dpp/fmt/format.h>
#include <exception>
#include <pqxx/connection.hxx>
#include <pqxx/nontransaction.hxx>
#include <pqxx/pqxx>
#include <queue>
#include <thread>

extern char* conn_string;

struct Query_request
{
    std::string query;
    Query_completion_callback cb;
};

static std::mutex mutex;
static std::thread thread;
static std::condition_variable cv;
static std::queue<Query_request> query_queue;

static pqxx::result start_query(pqxx::connection* conn, const std::string& query)
{
    pqxx::nontransaction w(*conn);
    return w.exec(query);
}

Database::Database() : terminating(false)
{
    conn = new pqxx::connection(conn_string);
    thread = std::thread(&Database::_worker, this);
}

void Database::_worker()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (true) {
        cv.wait(lock, [this]() { return !query_queue.empty() || terminating; });

        if (terminating) break;
        try {
            Query_request req = std::move(query_queue.front());
            query_queue.pop();

            pqxx::result res = start_query(conn, req.query);

            if (req.cb) {
                lock.unlock();
                req.cb(res);
                lock.lock();
            }
        } catch (pqxx::failure& e) {
            log_err("DATABASE", std::string("PQXX Failure: ") + e.what());
        } catch (std::exception& e) {
            log_err("DATABASE", e.what());
        }
    }
}

void Database::execute(const std::string& query, const Query_completion_callback& callback)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        query_queue.emplace(query, callback);
    }
    cv.notify_one();
}

pqxx::result Database::execute_sync(const std::string& query)
{
    std::lock_guard<std::mutex> lock(mutex);
    return start_query(conn, query);
}

Database::~Database()
{
    terminating = true;
    cv.notify_all();
    thread.join();
    delete conn;
}
