#pragma once
#include <string>
#include <random>
#include <cstdint>
#include <atomic>
#include <algorithm>
#include "../benchmark.hpp"
#include "../../utils/semaphore.hpp"
#include "../../utils/timer.hpp"
#include "../../utils/debug.hpp"

namespace CacheBenchmark {
    // No timestamp
    using CacheData = uint64_t;

    auto busy_sleep(uint64_t num_ns) -> void {
        auto start = get_timepoint();
        uint64_t now = 0;

        // Wait until time runs out
        do {
            now = get_timepoint();
        } while (get_duration(start, now) < num_ns);
    }

    template<typename T>
    inline auto benchmark_accessor(Semaphore& sem, T& map, uint64_t seed, const std::atomic<bool>& done, std::atomic<uint64_t>& num_accesses, uint64_t num_ids) -> void {
        sem.wait();

        std::mt19937 rng(seed);
        std::uniform_int_distribution<uint32_t> dist(0, num_ids);

        DBG(<< "[Accessor] Starting!" << std::endl);

        while (!done.load()) {
            auto index = dist(rng);

            // Access the cached resource
            map.access(index);
            num_accesses.fetch_add(1);

            // Sleep for 100 ns
            busy_sleep(10000);                                // TODO: Make this an argument
            DBG(<< "[Accessor] Waiting!" << std::endl);
        }
    }

    template<typename T>
    inline auto benchmark_cleaner(Semaphore& sem, T& map, const std::atomic<bool>& done, uint64_t num_ids) -> void {
        sem.wait();

        bool cleaning = false;

        DBG( << "[Cleaner] Starting!" << std::endl);

        for (uint64_t i = 0; !done.load(); i = (i + 1) % num_ids) {
            auto size = map.get_size();
            auto capacity = map.get_capacity();

            // Wait if we haven't reached 95% capacity and we aren't cleaning
            while (!cleaning && size < capacity - (capacity / 20)) {
                size = map.get_size();
                if (done.load())
                    return;
            }

            if (cleaning == false) {
                DBG( << "[Cleaner] Started cleaning!" << std::endl);
            }

            cleaning = true;
            map.erase(i);

            if (size < capacity - (capacity / 10)) {
                cleaning = false;
                DBG( << "[Cleaner] Stopped cleaning!" << std::endl);
            }
        }
    }

    template<typename T>
    inline auto benchmark_impl(uint64_t seed, uint64_t time_limit, uint64_t map_capacity, uint32_t num_threads) -> RunResult {
        T map(map_capacity);
        RunResult result{};

        Semaphore sem;
        Timer t;

        auto num_ids = map_capacity + (map_capacity / 5);

        // Cleaner thread
        std::atomic<bool> done = false;
        std::thread cleaner_thread(
            &benchmark_cleaner<T>,
            std::ref(sem),
            std::ref(map),
            std::cref(done),
            num_ids
        );

        // Accessor threads
        std::vector<std::thread> threads;
        std::atomic<uint64_t> num_accesses = 0;
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back(
                std::thread(
                    &benchmark_accessor<T>,
                    std::ref(sem),
                    std::ref(map),
                    seed + i,
                    std::cref(done),
                    std::ref(num_accesses),
                    num_ids
                )
            );
        }

        auto start = std::chrono::high_resolution_clock::now();
        auto end = start + std::chrono::milliseconds(time_limit);
        sem.notify_all();

        while (true) {
            auto current = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - current);
            auto ms = diff.count();

            std::cout << "Time left: " << diff.count() << "ms (size: " << map.get_size() << ", capacity: " << map.get_capacity() << ")" << std::endl;

            if (ms <= 0) {
                break;                                  // When we have reached the time limit, stop
            } else if (diff.count() <= 10) {
                continue;                               // Spin the last 10 milliseconds
            } else {
                std::this_thread::sleep_for(diff / 2);  // Wait otherwise
            }
        }

        // Finished
        done.store(true);

        // Clean up threads
        for (auto& th : threads) {
            th.join();
        }

        cleaner_thread.join();

        result.hash = 0;                        // No way to verify correctness
        result.time = num_accesses;             // TODO: Storing runs in time ...

        // We dont care about timing this thread

        return result;
    }

    template<typename T>
    inline auto run_benchmark(const std::string& impl, uint64_t seed, uint64_t time_limit, uint64_t map_capacity, uint32_t num_runs, uint32_t num_threads) -> BenchmarkResult {
        BenchmarkResult result{};

        result.impl = impl;
        result.correct = true;
        result.num_runs = num_runs;
        result.num_threads = num_threads;

        result.total_time = 0;
        result.min_time = std::numeric_limits<uint64_t>::max();
        result.max_time = std::numeric_limits<uint64_t>::min();

        result.hash = 0;

        for (uint32_t i = 0; i < num_runs; i++) {
            auto run_result = benchmark_impl<T>(seed, time_limit, map_capacity, num_threads);

            if (i == 0) {
                result.hash = run_result.hash;
            }

            if (run_result.hash != result.hash) {
                result.correct = false;
            }

            result.runs.push_back(run_result);          
            result.total_time += run_result.time;

            result.min_time = std::min(result.min_time, run_result.time);
            result.max_time = std::max(result.max_time, run_result.time);
        }

        result.avg_time = result.total_time / result.num_runs;
        result.mean_time = result.runs[result.num_runs / 2].time;

        return result;
    }
}