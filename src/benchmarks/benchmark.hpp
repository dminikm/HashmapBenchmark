#pragma once
#include <vector>
#include <string>
#include <cstdint>

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