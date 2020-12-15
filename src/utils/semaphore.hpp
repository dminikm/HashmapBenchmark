#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

class Semaphore {
    public:
        auto wait() -> void {
            std::unique_lock<std::mutex> lock(this->mut);
            var.wait(lock, [this]() { return this->ready; });
        }

        auto notify_all() -> void {
            std::lock_guard<std::mutex> lock(this->mut);
            this->ready = true;
            this->var.notify_all();
        }

    private:
        std::mutex mut;
        std::condition_variable var{};
        bool ready = false;
};

class IntSemaphore {
    public:
        auto wait() -> void {
            std::unique_lock<std::mutex> lock(this->mut);
            var.wait(lock, [this]() { 
                bool done = this->value > 0;

                if (done) {
                    this->value--;
                }

                return done;
            });
        }

        auto notify_one() -> void {
            std::lock_guard<std::mutex> lock(this->mut);
            this->value++;
        }

    private:
        std::mutex mut;
        std::condition_variable var{};
        int value = 0;
};