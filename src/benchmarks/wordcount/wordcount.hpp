#pragma once
#include <chrono>
#include <vector>
#include <limits>
#include <algorithm>
#include <functional>
#include "interface.hpp"
#include "../../utils/semaphore.hpp"
#include "../../utils/timer.hpp"

namespace WordCountBenchmark {
    using WordFile = std::vector<std::string>;

    struct RunResult {
        uint64_t time;
        uint64_t hash;
    };

    struct BenchmarkResult {
        std::string impl;

        std::vector<RunResult> runs;

        uint64_t total_time;
        uint64_t avg_time;
        uint64_t min_time;
        uint64_t max_time;
        uint64_t mean_time;

        uint64_t hash;

        uint32_t num_threads;
        uint32_t num_runs;

        bool correct;
    };

    template<typename T>
    inline auto benchmark_count_part(Semaphore& semaphore, const WordFile& file, T& map, uint32_t start, uint32_t end) -> void {
        // Wait for test start
        semaphore.wait();

        for (auto i = start; i < end; i++) {
            auto& line = file[i];

            uint32_t word_start = 0;
            for (auto o = 0; o < line.size(); o++) {
                auto ch = line[o];

                if (!std::isalnum(ch)) {
                    uint32_t len = o - word_start;

                    // Empty word
                    if (len == 0) {
                        word_start = o;
                        break;
                    }

                    map.increase_or_insert(std::string_view(line.c_str() + word_start, len), 1);
                    word_start = o + 1;
                }
            }
        }
    }

    template<typename T>
    inline uint64_t hash_whole_map(T& map) {
        auto kvs = map.get_key_value_pairs();
        uint64_t hash = kvs.size();

        for (auto& [key, value] : kvs) {
            auto key_hash = std::hash<std::string_view>{}(key);
            auto value_hash = std::hash<uint32_t>{}(value);

            hash ^= key_hash + value_hash + 0x9E3779B9 + (hash << 6) + (hash >> 2);
        }

        return hash;
    }

    template<typename T>
    inline auto benchmark_impl(const WordFile& file, uint32_t num_threads) -> RunResult {
        T map;
        Semaphore sem;
        
        RunResult result;
        auto even_split = file.size() / num_threads;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (auto i = 0; i < num_threads; i++) {
            auto start = i * even_split;
            auto end = (i == num_threads - 1) ? file.size() : ((i + 1) * even_split);

            threads.emplace_back(
                std::thread(
                    &benchmark_count_part<T>,
                    std::ref(sem),
                    std::cref(file),
                    std::ref(map),
                    start,
                    end
                )
            );
        }

        // Start timer
        Timer t;
        t.start();

        // Wake up threads
        sem.notify_all();

        // Join all threads
        for (auto& th : threads) {
            th.join();
        }

        // End timer
        t.end();

        result.hash = hash_whole_map<T>(map);
        result.time = t.get_duration();

        return result;
    }

    template<typename T>
    inline auto run_benchmark(std::string impl, const WordFile& file, uint32_t num_runs, uint32_t num_threads) -> BenchmarkResult {
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
            auto run_result = benchmark_impl<T>(file, num_threads);

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