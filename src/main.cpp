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

auto main(int argc, const char** argv) -> int {
    cxxopts::Options options("HashmapBenchmark", "Benchmark multiple concurrent hashmaps!");

    options.add_options()
        ("t,threads", "Number of threads", cxxopts::value<uint32_t>()->default_value("16"))
        ("r,runs", "Number of runs per hashmap", cxxopts::value<uint32_t>()->default_value("10"))
        ("d,dataset", "Path to the used dataset", cxxopts::value<std::string>()->default_value("../data/test.ft.txt.out"))
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

    auto benchmark_result = WordCountBenchmark::run_benchmark(WordCountBenchmark::tbbmap_count_words, *file, num_runs, num_threads);

    std::cout << "Benchmark result:" << std::endl;
    std::cout << "Correct:   " << benchmark_result.correct << std::endl;
    std::cout << "Hash:      " << benchmark_result.hash << std::endl;
    std::cout << "Runtime:   " << benchmark_result.total_time << std::endl;
    std::cout << "Min time:  " << benchmark_result.min_time << std::endl;
    std::cout << "Max time:  " << benchmark_result.max_time << std::endl;
    std::cout << "Avg time:  " << benchmark_result.avg_time << std::endl;
    std::cout << "Mean time: " << benchmark_result.mean_time << std::endl;
 
    return 0;
}