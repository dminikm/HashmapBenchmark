#pragma once
#include <chrono>
#include <vector>
#include <limits>
#include <algorithm>
#include <functional>
#include "interface.hpp"
#include "../../utils/semaphore.hpp"
#include "../../utils/timer.hpp"
#include "../benchmark.hpp"

namespace WordCountBenchmark {
    using WordFile = std::vector<std::string>;

    auto load_file(const std::string& path) -> std::optional<WordFile> {
        std::ifstream file(path);

        if (!file.is_open()) {
            return {};
        }

        std::vector<std::string> result;

        std::string line;
        while (std::getline(file, line)) {
            result.push_back(std::move(line));
        }

        return result;
    }

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

            // In case this line ends with a valid letter, we need to add it to the hash map
            auto o = line.size() - 1;
            auto len = o - word_start;

            // Word was already added
            if (word_start >= o) {
                continue;
            }

            // Empty word check
            if (len > 0) {
                map.increase_or_insert(std::string_view(line.c_str() + word_start, len), 1);
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
        result.value = t.get_duration();

        return result;
    }

    template<typename T>
    inline auto run_benchmark(std::string impl, const WordFile& file, uint32_t num_runs, uint32_t num_threads) -> BenchmarkResult {
        BenchmarkResult result{};

        result.impl = impl;
        result.value_unit = "ns";
        result.correct = true;

        result.num_runs = num_runs;
        result.num_threads = num_threads;
        
        result.total_value = 0;
        result.min_value = std::numeric_limits<uint64_t>::max();
        result.max_value = std::numeric_limits<uint64_t>::min();

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
            
            result.total_value += run_result.value;

            result.min_value = std::min(result.min_value, run_result.value);
            result.max_value = std::max(result.max_value, run_result.value);
        }

        result.avg_value = result.total_value / result.num_runs;
        result.mean_value = result.runs[result.num_runs / 2].value;

        return result;
    }
}