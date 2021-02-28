#pragma once
#include <vector>
#include <string>
#include <cstdint>

struct RunResult {
    uint64_t value;
    uint64_t hash;
};

struct BenchmarkResult {
    std::string impl;

    std::vector<RunResult> runs;

    std::string value_unit;
    uint64_t total_value;
    uint64_t avg_value;
    uint64_t min_value;
    uint64_t max_value;
    uint64_t mean_value;

    uint64_t hash;

    uint32_t num_threads;
    uint32_t num_runs;

    bool correct;
};