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

        for (auto i = start; i < end; i++) {
            auto& item = dataset_b[i];
            auto value = map.get(std::get<1>(item));

            queue->push(std::make_tuple(
                std::get<0>(value),
                std::get<1>(value),
                std::get<0>(item),
                std::get<1>(item),
                std::get<2>(item)
            ));
        }

        // Stop worker thread
        queue->finish();
    }

    template <typename T>
    inline auto hash(T value) -> uint64_t {
        return std::hash<T>{}(value);
    }

    inline auto hash_combine(uint64_t hash1, uint64_t hash2) -> uint64_t {
        return hash1 ^ (hash2 + 0x9E3779B9 + (hash1 << 6) + (hash1 >> 2));
    }

    inline auto hash_result(HashJoinResultValue value) -> uint64_t {
        return std::apply([](auto&& ... x) {
            uint64_t h = 0;
            ((h = ... = hash_combine(h, hash(x))));
            return h;
        }, value);
    }

    inline auto benchmark_probe_part_handler(Semaphore& sem, std::shared_ptr<WorkQueue<HashJoinResultValue>> queue, std::shared_ptr<uint64_t> hash_ptr) -> void {
        sem.wait();

        // DEBUG:
        //std::cout << "PK1\t\tPK2\t\tFK2\t\tHASH" << std::endl;

        auto res = queue->pop();
        auto h = hash_result(res.value);

        while (!res.finished)
        {
            // TODO: Hash combine
            res = queue->pop();
            h = hash_combine(h, hash_result(res.value));
            //std::cout << std::get<0>(res.value) << "\t\t" << std::get<2>(res.value) << "\t\t" << std::get<3>(res.value) << "\t\t" << std::hex << h << std::dec << std::endl;
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
            std::vector<std::tuple<std::thread, std::thread, std::shared_ptr<uint64_t>>> threads;

            for (auto i = 0; i < num_threads; i++) {
                auto start = i * even_split;
                auto end = (i == num_threads - 1) ? dataset_b.size() : ((i + 1) * even_split);

                auto queue = std::make_shared<WorkQueue<HashJoinResultValue>>();
                auto hash = std::make_shared<uint64_t>(0);

                threads.push_back(std::make_tuple(
                    std::thread(
                        &benchmark_probe_part_prober<T>,
                        std::ref(sem),
                        std::cref(dataset_b),
                        std::ref(map),
                        queue,
                        start,
                        end
                    ), std::thread(
                        &benchmark_probe_part_handler,
                        std::ref(sem),
                        queue,
                        hash
                    ),
                    hash
                ));
            }

            t.start();
            sem.notify_all();

            uint64_t hash = 0;

            for (auto& [t1, t2, h] : threads) {
                t1.join();
                t2.join();

                hash = hash_combine(hash, *h);
            }

            t.end();

            result.hash = hash;
            result.time = t.get_duration() + build_duration;
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