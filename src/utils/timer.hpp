#pragma once
#include <cstdint>

#ifdef TEST_NO_WINDOWS
#include <Windows.h>

class Timer {
    public:
        auto start() -> void {
            this->start_ns = this->get_timepoint();
            this->end_ns = this->start_ns;
            this->running = true;
        }

        auto end() -> void {
            this->end_ns = this->get_timepoint();
            this->running = false;
        }

        auto get_duration() -> uint64_t {
            if (this->running) {
                this->end();
                this->running = true;
            }

            LARGE_INTEGER frequency{};
            QueryPerformanceFrequency(&frequency);

            uint64_t ns_per_count = (uint64_t)1e9 / frequency.QuadPart;

            auto difference = this->end_ns - this->start_ns;
            return difference * ns_per_count;
        }

        auto is_running() -> bool {
            return this->running;
        }

    private:
        auto get_timepoint() -> uint64_t {
            LARGE_INTEGER timepoint{};
            QueryPerformanceCounter(&timepoint);

            return timepoint.QuadPart;
        }

    private:
        uint64_t start_ns = 0;
        uint64_t end_ns = 0;
        bool running = true;
};
#else
#include <chrono>

class Timer {
    public:
        auto start() -> void {
            this->start_pt = this->get_timepoint();
            this->end_pt = this->start_pt;
            this->running = true;
        }

        auto end() -> void {
            this->end_pt = this->get_timepoint();
            this->running = false;
        }

        auto get_duration() -> uint64_t {
            if (this->running) {
                this->end();
                this->running = true;
            }

            auto difference = this->end_pt - this->start_pt;
            return std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count();
        }

        auto is_running() -> bool {
            return this->running;
        }

    private:
        auto get_timepoint() -> std::chrono::time_point<std::chrono::high_resolution_clock> {
            return std::chrono::high_resolution_clock::now();
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start_pt;
        std::chrono::time_point<std::chrono::high_resolution_clock> end_pt;
        bool running = true;
};
#endif