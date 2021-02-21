#pragma once
#include <cstdint>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>

auto get_timepoint() -> uint64_t {
    LARGE_INTEGER timepoint{};
    QueryPerformanceCounter(&timepoint);

    return timepoint.QuadPart;
}

auto get_duration(uint64_t start_timepoint, uint64_t end_timepoint) -> uint64_t {
    LARGE_INTEGER frequency{};
    QueryPerformanceFrequency(&frequency);

    uint64_t ns_per_count = (uint64_t)1e9 / frequency.QuadPart;

    auto difference = end_timepoint - start_timepoint;
    return difference * ns_per_count;
}

class Timer {
    public:
        auto start() -> void {
            this->start_ns = get_timepoint();
            this->end_ns = this->start_ns;
            this->running = true;
        }

        auto end() -> void {
            this->end_ns = get_timepoint();
            this->running = false;
        }

        auto get_duration() -> uint64_t {
            if (this->running) {
                this->end();
                this->running = true;
            }

            return ::get_duration(this->start_ns, this->end_ns);
        }

        auto is_running() -> bool {
            return this->running;
        }

    private:
        uint64_t start_ns = 0;
        uint64_t end_ns = 0;
        bool running = true;
};
#else
#include <chrono>

auto get_timepoint() -> std::chrono::time_point<std::chrono::high_resolution_clock> {
    return std::chrono::high_resolution_clock::now();
}

auto get_duration(uint64_t start_timepoint, uint64_t end_timepoint) -> uint64_t {
    return end_timepoint - start_timepoint
}

class Timer {
    public:
        auto start() -> void {
            this->start_pt = get_timepoint();
            this->end_pt = this->start_pt;
            this->running = true;
        }

        auto end() -> void {
            this->end_pt = get_timepoint();
            this->running = false;
        }

        auto get_duration() -> uint64_t {
            if (this->running) {
                this->end();
                this->running = true;
            }

            return this->end_pt - this->start_pt
        }

        auto is_running() -> bool {
            return this->running;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start_pt{};
        std::chrono::time_point<std::chrono::high_resolution_clock> end_pt{};
        bool running = true;
};
#endif