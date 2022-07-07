#include "../logger.h"
#include "events.h"

void event_log(const dpp::log_t& event)
{
    switch (event.severity) {
    case dpp::ll_debug:
        log_debug("CORE", event.message);
        break;
    case dpp::ll_warning:
        log_warn("CORE", event.message);
        break;
    case dpp::ll_error:
        log_err("CORE", event.message);
        break;
    case dpp::ll_critical:
        log_crit("CORE", event.message);
        break;
    default:
        log_info("CORE", event.message);
        break;
    }
}