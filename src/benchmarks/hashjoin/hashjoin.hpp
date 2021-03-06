#pragma once
#include <chrono>
#include <vector>
#include <limits>
#include <algorithm>
#include <functional>
#include <tuple>
#include <fstream>
#include "../../utils/semaphore.hpp"
#include "../../utils/timer.hpp"
#include "../benchmark.hpp"

namespace HashJoinBenchmark {
    using DatasetAValue = std::tuple<uint32_t, std::string>;
    using DatasetA = std::vector<DatasetAValue>;

    using DatasetBValue = std::tuple<uint32_t, uint32_t, std::string>;
    using DatasetB = std::vector<DatasetBValue>;

    using HashJoinResultValue = std::tuple<uint32_t, std::string, uint32_t, uint32_t, std::string>;
    using HashJoinResult = std::vector<HashJoinResultValue>;

    inline auto load_dataset_a(std::string& file_name) -> DatasetA {
        std::ifstream file(file_name);

        if (!file.is_open()) {
            return {};
        }

        DatasetA result{};

        std::string line;
        while (std::getline(file, line)) {
            auto delimiterPos = line.find_first_of(';');

            result.emplace_back(std::make_pair(
                std::atoi(line.substr(0, delimiterPos).c_str()),
                line.substr(delimiterPos + 1)
            ));
        }

        return result;
    }

    inline auto load_dataset_b(std::string& file_name) -> DatasetB {
        std::ifstream file(file_name);

        if (!file.is_open()) {
            return {};
        }

        DatasetB result{};

        std::string line;
        while (std::getline(file, line)) {
            auto delimiterPos = line.find_first_of(';');
            auto delimiterPos2 = line.find_first_of(';', delimiterPos + 1);

            result.emplace_back(std::make_tuple(
                std::atoi(line.substr(0, delimiterPos).c_str()),
                std::atoi(line.substr(delimiterPos + 1, (delimiterPos2 - delimiterPos) - 1).c_str()),
                line.substr(delimiterPos2 + 1)
            ));
        }

        return result;
    }

    template<typename T>
    inline auto benchmark_build_part(Semaphore& sem, const DatasetA& dataset_a, T& map, uint32_t start, uint32_t end) -> void {
        sem.wait();

        for (auto i = start; i < end; i++) {
            auto& item = dataset_a[i];
            map.insert(std::get<0>(item), item);
        }
    }

    template <typename T>
    inline auto hash(const T& value) -> uint64_t {
        return std::hash<T>{}(value);
    }

    inline auto hash_combine(uint64_t hash1, uint64_t hash2) -> uint64_t {
        return hash1 ^ (hash2 + 0x9E3779B9 + (hash1 << 6) + (hash1 >> 2));
    }

    inline auto hash_result(const HashJoinResultValue& value) -> uint64_t {
        return std::apply([](auto&& ... x) {
            uint64_t h = 0;
            ((h = ... = hash_combine(h, hash(x))));
            return h;
        }, value);
    }

    template<typename T>
    inline auto benchmark_probe_part(Semaphore& sem, const DatasetB& dataset_b, T& map, uint32_t start, uint32_t end, std::shared_ptr<uint64_t> hash_ptr) -> void {        
        uint64_t h = 0;
        sem.wait();

        for (auto i = start; i < end; i++) {
            auto& item = dataset_b[i];
            auto value = map.get(std::get<1>(item));

            h = hash_combine(h, hash_result(std::make_tuple(
                std::get<0>(value),
                std::get<1>(value),
                std::get<0>(item),
                std::get<1>(item),
                std::get<2>(item)
            )));
        }

        *hash_ptr = h;
    }

    template<typename T>
    inline auto benchmark_impl(const DatasetA& dataset_a, const DatasetB& dataset_b, uint32_t num_threads) -> RunResult {
        T map;

        RunResult result{};

        uint64_t build_duration = 0;
        // Build phase
        {
            Timer t;
            Semaphore sem;
            auto even_split = dataset_a.size() / num_threads;

            std::vector<std::thread> threads;
            threads.reserve(num_threads);

            for (auto i = 0; i < num_threads; i++) {
                auto start = i * even_split;
                auto end = (i == num_threads - 1) ? dataset_a.size() : ((i + 1) * even_split);

                threads.emplace_back(
                    &benchmark_build_part<T>,
                    std::ref(sem),
                    std::cref(dataset_a),
                    std::ref(map),
                    start,
                    end
                );
            }

            t.start();
            sem.notify_all();

            for (auto& th : threads) {
                th.join();
            }

            t.end();
            build_duration = t.get_duration();
        }

        // Probe phase
        {
            Timer t;
            Semaphore sem;

            auto even_split = dataset_b.size() / num_threads;
            std::vector<std::tuple<std::thread, std::shared_ptr<uint64_t>>> threads;

            for (auto i = 0; i < num_threads; i++) {
                auto start = i * even_split;
                auto end = (i == num_threads - 1) ? dataset_b.size() : ((i + 1) * even_split);

                auto hash = std::make_shared<uint64_t>(0);

                threads.push_back(std::make_tuple(
                    std::thread(
                        &benchmark_probe_part<T>,
                        std::ref(sem),
                        std::cref(dataset_b),
                        std::ref(map),
                        start,
                        end,
                        hash
                    ),
                    hash
                ));
            }

            t.start();
            sem.notify_all();

            uint64_t hash = 0;

            for (auto& [t1, h] : threads) {
                t1.join();
                hash = hash_combine(hash, *h);
            }

            t.end();

            result.hash = hash;
            result.value = t.get_duration() + build_duration;
        }

        return result;
    }

    template<typename T>
    inline auto run_benchmark(const std::string& impl, const DatasetA& dataset_a, const DatasetB& dataset_b, uint32_t num_runs, uint32_t num_threads) -> BenchmarkResult {
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
            std::cout << "Starting iteration " << (i + 1) << "/" << num_runs << std::endl;
            auto run_result = benchmark_impl<T>(dataset_a, dataset_b, num_threads);

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