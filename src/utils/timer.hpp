#pragma once

#ifdef _WIN32 || __CYGWIN__
#include <cstdint>
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

#endif