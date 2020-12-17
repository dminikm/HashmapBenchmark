#pragma once
#include "semaphore.hpp"
#include <queue>
#include <mutex>

template<typename T>
struct WorkQueuePopResult {
    bool finished;
    T value;
};

template<typename T>
class WorkQueue {
    public:
        WorkQueue() {

        }

        auto pop() -> WorkQueuePopResult<T> {
            // Wait until item becomes available
            this->sem.wait();
            std::lock_guard<std::mutex> lock(this->mtx);

            auto value = this->inner.front();
            this->inner.pop();

            return {
                this->finished && this->inner.size() == 0,
                value
            };
        }

        auto push(T item) -> void {
            std::lock_guard<std::mutex> lock(this->mtx);
            this->inner.push(item);
            this->sem.notify_one();
        }

        auto finish() -> void {
            std::lock_guard<std::mutex> lock(this->mtx);
            this->finished = true;
        }

        auto is_finished() -> bool {
            return this->finished;
        }

    private:
        std::queue<T> inner;
        std::mutex mtx;
        IntSemaphore sem;

        bool finished = false;
};