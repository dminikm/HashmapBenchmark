#pragma once
#include <chrono>
#include <vector>
#include <limits>
#include <algorithm>
#include <functional>
#include "../../utils/semaphore.hpp"
#include "../../utils/timer.hpp"

namespace WordCountBenchmark {
    using WordFile = std::vector<std::string>;

    struct RunResult {
        uint64_t time;
        uint64_t hash;
    };

    struct BenchmarkResult {
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

    using RunFunction = RunResult(const WordFile& file, uint32_t num_threads);
    inline auto run_benchmark(std::function<RunFunction> fn, const WordFile& file, uint32_t num_runs, uint32_t num_threads) -> BenchmarkResult {
        BenchmarkResult result{};

        result.correct = true;

        result.num_runs = num_runs;
        result.num_threads = num_threads;
        
        result.total_time = 0;
        result.min_time = std::numeric_limits<uint64_t>::max();
        result.max_time = std::numeric_limits<uint64_t>::min();

        result.hash = 0;

        for (uint32_t i = 0; i < num_runs; i++) {
            auto run_result = fn(file, num_threads);

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