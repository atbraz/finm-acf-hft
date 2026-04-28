#pragma once

#include <chrono>
#include <vector>

class Timer {
public:
    void start() { t0_ = std::chrono::high_resolution_clock::now(); }
    long long stop() const {
        auto t1 = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0_).count();
    }
private:
    std::chrono::high_resolution_clock::time_point t0_{};
};

class ScopedTimer {
public:
    explicit ScopedTimer(std::vector<long long>& sink) : sink_(sink) {
        t0_ = std::chrono::high_resolution_clock::now();
    }
    ~ScopedTimer() {
        auto t1 = std::chrono::high_resolution_clock::now();
        sink_.push_back(
            std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0_).count());
    }
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
private:
    std::vector<long long>& sink_;
    std::chrono::high_resolution_clock::time_point t0_;
};
