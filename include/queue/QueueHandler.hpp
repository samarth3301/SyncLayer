#pragma once

#include <queue>
#include "../tracker/ChangeEvent.hpp"

namespace SyncLayer {
namespace DB { class DBConnection; }
}

namespace SyncLayer::Queue {

class QueueHandler {
public:
    void enqueue(const SyncLayer::Tracker::ChangeEvent& event);
    void drainTo(SyncLayer::DB::DBConnection* target);

private:
    std::queue<SyncLayer::Tracker::ChangeEvent> q_;
};

} // namespace SyncLayer::Queue


