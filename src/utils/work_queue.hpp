#pragma once
#include "semaphore.hpp"
#include <queue>
#include <mutex>
#include <tbb/concurrent_queue.h>
#include <atomic>

template<typename T>
struct WorkQueueWrapper {
    bool finished;
    T value;
};

template<typename T>
class WorkQueue {
    public:
        WorkQueue() {

        }

        auto pop() -> WorkQueueWrapper<T> {
            WorkQueueWrapper<T> value;

            // Busy loop
            while (!this->queue.try_pop(value)) {}
            return value;
        }

        auto push(T&& item, bool finished = false) -> void {
            queue.emplace(WorkQueueWrapper<T> { .finished = finished, .value = item });
        }

    private:
        tbb::concurrent_queue<WorkQueueWrapper<T>> queue{};
};