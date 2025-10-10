#include "queue/QueueHandler.hpp"
#include "tracker/TableTracker.hpp"
#include "db/DBConnection.hpp"
#include <spdlog/spdlog.h>

namespace SyncLayer::Queue {

void QueueHandler::enqueue(const SyncLayer::Tracker::ChangeEvent& event)
{
    q_.push(event);
}

void QueueHandler::drainTo(SyncLayer::DB::DBConnection* target)
{
    // Placeholder: execute SQL against target using libpq
    int drained = 0;
    while (!q_.empty()) {
        const auto& ev = q_.front();
        spdlog::info("Replicating {} on {}: {}", ev.operation, ev.table, ev.payloadJson);
        q_.pop();
        ++drained;
    }
    spdlog::info("Drained {} events to target", drained);
}

} // namespace SyncLayer::Queue


