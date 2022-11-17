#pragma once
#include "bot.h"

#define DEFAULT_COLLECTOR_TIMEOUT 60

class Bot;

extern "C" typedef struct None
{
} None;

/**
 * @brief Reimplemented collector from DPP with some modifications.
 * It does not attach to the event
 * Make sure to attach it to the event that produce the object type.
 * @tparam T object type this collector will store
 */
template<class T> class Collector
{
public:
    Collector(Bot* bot,
              uint64_t duration = DEFAULT_COLLECTOR_TIMEOUT,
              bool store_item   = true,
              bool auto_reset   = true)
        : mutex(std::mutex()), bot_(bot), duration_(duration), auto_reset_(auto_reset), store_item_(store_item)
    {
        _timer = bot_->start_timer(_Delete {this}, duration_);
    }

    bool terminating = false;
    mutable std::mutex mutex;

    void destroy()
    {
        {
            std::lock_guard lock(mutex);
            on_end(_stored);
        }
    };

    void execute(const T& item)
    {
        if (mutex.try_lock()) {
            if (on_collect(item)) {
                if (store_item_) _stored.emplace_back(item);
                if (auto_reset_) time_reset();
            }
            mutex.unlock();
        }
        if (terminating) destroy();
    }

    void time_reset(uint64_t duration = 0)
    {
        bot_->stop_timer(_timer);
        _timer = bot_->start_timer(_Delete {this}, duration ? duration : duration_);
    }

    virtual bool on_collect(const T& item) = 0;

    virtual void on_end(const std::vector<T>& list) {}

    virtual ~Collector() { 
        bot_->stop_timer(_timer);
        if (!terminating) destroy();
    }

protected:
    bool auto_reset_;
    bool store_item_;
    Bot* bot_;
    uint64_t duration_;

private:
    struct _Delete
    {
        Collector<T>* c;
        void operator()(dpp::timer)
        {
            c->destroy();
        }
    };

    std::vector<T> _stored;
    dpp::timer _timer;
};
