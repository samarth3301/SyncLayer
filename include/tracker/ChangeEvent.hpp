#pragma once

#include <string>

namespace SyncLayer {
namespace Tracker {

struct ChangeEvent {
    std::string table;
    std::string operation; // insert/update/delete
    std::string payloadJson; // serialized row change
};

} // namespace Tracker
} // namespace SyncLayer


