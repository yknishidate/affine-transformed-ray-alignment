#pragma once
#include <chrono>
#include <functional>

namespace Time
{
    inline double measure(const std::function<void()>& func)
    {
        auto startTime = std::chrono::steady_clock::now();
        func();
        auto endTime = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000.0;
    }
};
