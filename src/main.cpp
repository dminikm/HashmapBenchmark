#include <cstdint>
#include <cxxopts.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <thread>
#include <cctype>

#include "benchmarks/wordcount/libcuckoo.hpp"
#include "benchmarks/wordcount/stdmap.hpp"
#include "benchmarks/wordcount/tbbmap.hpp"
#include "utils/json_serializer.hpp"

auto load_file(const std::string& path) -> std::optional<WordCountBenchmark::WordFile> {
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

auto write_file(const std::string& path, const std::string& text) -> void {
    std::ofstream file(path);

    if (!file.is_open()) {
        std::cerr << "Could not open file \"" << path << "\"!" << std::endl;
        std::exit(-1);
    }

    file << text;
}

auto main(int argc, const char** argv) -> int {
    cxxopts::Options options("HashmapBenchmark", "Benchmark multiple concurrent hashmaps!");

    options.add_options()
        ("t,threads", "Number of threads", cxxopts::value<uint32_t>()->default_value("16"))
        ("r,runs", "Number of runs per hashmap", cxxopts::value<uint32_t>()->default_value("10"))
        ("d,dataset", "Path to the used dataset", cxxopts::value<std::string>()->default_value("../data/test.ft.txt.out"))
        ("j,json", "Path to JSON output", cxxopts::value<std::string>()->implicit_value("json_out.txt"))
        ("w,wordcount", "Run the wordcount benchmark", cxxopts::value<std::string>()->implicit_value("std"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    auto num_threads = result["threads"].as<uint32_t>();
    auto num_runs = result["runs"].as<uint32_t>();
    auto dataset_path = result["dataset"].as<std::string>();

    auto file = load_file(dataset_path);

    if (!file) {
        std::cout << "Dataset '" << dataset_path << "' does not exist, aborting!" << std::endl;
        return -1;
    }

    WordCountBenchmark::BenchmarkResult benchmark_result;

    for (int i = 1; i <= num_threads; i++) {
        std::cout << "Benchmarking libcuckoo!" << std::endl;
        benchmark_result = WordCountBenchmark::run_benchmark<WordCountBenchmark::CuckooMap>("libcuckoo", *file, num_runs, i);
    
        write_file("profile-runs/" + benchmark_result.impl + "_" + std::to_string(i) + "t_" + std::to_string(num_runs) + "r.json", JSONSerializer::serialize_benchmark_result(benchmark_result));

        std::cout << "Benchmarking TBB concurrent_unordered_map!" << std::endl;
        benchmark_result = WordCountBenchmark::run_benchmark<WordCountBenchmark::TBBUnorderedMap>("tbb_unordered_map", *file, num_runs, i);

        write_file("profile-runs/" + benchmark_result.impl + "_" + std::to_string(i) + "t_" + std::to_string(num_runs) + "r.json", JSONSerializer::serialize_benchmark_result(benchmark_result));
    
        std::cout << "Benchmarking TBB concurrent_hash_map!" << std::endl;
        benchmark_result = WordCountBenchmark::run_benchmark<WordCountBenchmark::TBBHashMap>("tbb_hash_map", *file, num_runs, i);

        write_file("profile-runs/" + benchmark_result.impl + "_" + std::to_string(i) + "t_" + std::to_string(num_runs) + "r.json", JSONSerializer::serialize_benchmark_result(benchmark_result));
    
        std::cout << "Benchmarking Blocking STD!" << std::endl;
        benchmark_result = WordCountBenchmark::run_benchmark<WordCountBenchmark::BlockingSTDMap>("blocking-std", *file, num_runs, i);

        write_file("profile-runs/" + benchmark_result.impl + "_" + std::to_string(i) + "t_" + std::to_string(num_runs) + "r.json", JSONSerializer::serialize_benchmark_result(benchmark_result));
    }
 
    return 0;
}