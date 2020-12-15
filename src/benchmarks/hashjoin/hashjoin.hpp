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
#include "../../utils/work_queue.hpp"
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

        DatasetA result;

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

        DatasetB result;

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

    template<typename T>
    inline auto benchmark_probe_part_prober(Semaphore& sem, const DatasetB& dataset_b, T& map, std::shared_ptr<WorkQueue<HashJoinResultValue>> queue, uint32_t start, uint32_t end) -> void {
        sem.wait();
    }

    inline auto benchmark_probe_part_handler(Semaphore& sem, std::shared_ptr<WorkQueue<HashJoinResultValue>> queue) -> uint64_t {
        sem.wait();

        auto res = queue->pop();
        do
        {
            // TODO: Hash combine
        } while (!res.finished);

        return 0;
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

            using SharedThread = std::shared_ptr<std::thread>;
            std::vector<std::pair<SharedThread, SharedThread>> threads;

            for (auto i = 0; i < num_threads; i++) {
                auto start = i * even_split;
                auto end = (i == num_threads - 1) ? dataset_b.size() : ((i + 1) * even_split);

                auto queue = std::make_shared<WorkQueue<HashJoinResultValue>>();
            }
        }

        return result;
    }

    template<typename T>
    inline auto run_benchmark(const std::string& impl, const DatasetA& dataset_a, const DatasetB& dataset_b, uint32_t num_runs, uint32_t num_threads) -> BenchmarkResult {
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
            auto run_result = benchmark_impl<T>(dataset_a, dataset_b, num_threads);

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