#pragma once

#include <chrono>
#include <functional>

namespace SyncLayer::Utils {

class Retry {
public:
    static void withExponentialBackoff(int maxAttempts, std::function<bool(int)> attemptFn);
};

} // namespace SyncLayer::Utils


