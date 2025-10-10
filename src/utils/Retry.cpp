#include "utils/Retry.hpp"
#include <thread>

namespace SyncLayer::Utils {

void Retry::withExponentialBackoff(int maxAttempts, std::function<bool(int)> attemptFn)
{
    int attempt = 0;
    std::chrono::milliseconds delay(100);
    while (attempt < maxAttempts) {
        if (attemptFn(attempt + 1)) return;
        std::this_thread::sleep_for(delay);
        delay *= 2;
        ++attempt;
    }
}

} // namespace SyncLayer::Utils


