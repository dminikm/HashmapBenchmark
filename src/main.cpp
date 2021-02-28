#include <cstdint>
#include <cxxopts.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <thread>
#include <cctype>

#include "utils/json_serializer.hpp"
#include "benchmarks/benchmarks.hpp"

auto write_file(const std::string& path, const std::string& text) -> void {
    std::ofstream file(path);

    if (!file.is_open()) {
        std::cerr << "Could not open file \"" << path << "\"!" << std::endl;
        std::exit(-1);
    }

    file << text;
}

auto main_wordcount(int argc, const char** argv) -> BenchmarkResult {
    cxxopts::Options options("HashmapBenchmark wordcount", "Benchmark multiple concurrent hashmaps (WordCount benchmark)!");

    options.add_options()
        ("t,threads", "Number of threads", cxxopts::value<uint32_t>()->default_value("16"))
        ("r,runs", "Number of runs per hashmap", cxxopts::value<uint32_t>()->default_value("10"))
        ("d,dataset", "Path to the used dataset", cxxopts::value<std::string>()->default_value("../data/test.ft.txt.out"))
        ("i,implementation", "Map implementation to use", cxxopts::value<std::string>()->implicit_value("std"))
        ("j,json", "Path to JSON output", cxxopts::value<std::string>()->implicit_value("json_out.json"))
        ("h,help", "Print usage");

    options.allow_unrecognised_options();
    auto result = options.parse(argc, argv);

    if (result.count("help") > 0) {
        std::cout << options.help() << std::endl;
        std::exit(0);
    }

    auto num_threads = result["threads"].as<uint32_t>();
    auto num_runs = result["runs"].as<uint32_t>();
    auto dataset_path = result["dataset"].as<std::string>();

    auto file = WordCountBenchmark::load_file(dataset_path);

    if (!file) {
        std::cout << "Dataset '" << dataset_path << "' does not exist, aborting!" << std::endl;
        std::exit(-1);
    }

    if (result.count("implementation") == 0) {
        std::cout << "No map implementation selected" << std::endl;
        std::cout << options.help() << std::endl;
        std::exit(-1);
    }

    auto benchmark_impl_name = result["implementation"].as<std::string>();

    if (benchmark_impl_name == "libcuckoo") {
        std::cout << "Benchmarking libcuckoo!" << std::endl;
        return WordCountBenchmark::run_benchmark<WordCountBenchmark::CuckooMap>(benchmark_impl_name, *file, num_runs, num_threads);
    } else if (benchmark_impl_name == "tbb-unordered") {
        std::cout << "Benchmarking TBB concurrent_unordered_map!" << std::endl;
        return WordCountBenchmark::run_benchmark<WordCountBenchmark::TBBUnorderedMap>(benchmark_impl_name, *file, num_runs, num_threads);
    } else if (benchmark_impl_name == "tbb-hash") {
        std::cout << "Benchmarking TBB concurrent_hash_map!" << std::endl;
        return WordCountBenchmark::run_benchmark<WordCountBenchmark::TBBHashMap>(benchmark_impl_name, *file, num_runs, num_threads);
    } else if (benchmark_impl_name == "std-blocking") {
        std::cout << "Benchmarking Blocking STD!" << std::endl;
        return WordCountBenchmark::run_benchmark<WordCountBenchmark::BlockingSTDMap>(benchmark_impl_name, *file, num_runs, num_threads);
    } else {
        std::cerr << "Unknown implementation " << benchmark_impl_name << std::endl;
        std::exit(-1);
    }
}

auto main_hashjoin(int argc, const char** argv) -> BenchmarkResult {
    cxxopts::Options options("HashmapBenchmark hashjoin", "Benchmark multiple concurrent hashmaps (HashJoin benchmark)!");

    options.add_options()
        ("t,threads", "Number of threads", cxxopts::value<uint32_t>()->default_value("16"))
        ("r,runs", "Number of runs per hashmap", cxxopts::value<uint32_t>()->default_value("10"))
        ("a,dataseta", "Path to the used dataset A", cxxopts::value<std::string>()->default_value("../data/hash_join_smaller.txt"))
        ("b,datasetb", "Path to the used dataset B", cxxopts::value<std::string>()->default_value("../data/hash_join_larger.txt"))
        ("j,json", "Path to JSON output", cxxopts::value<std::string>()->implicit_value("json_out.json"))
        ("i,implementation", "Map implementation to use", cxxopts::value<std::string>()->implicit_value("std-blocking"))
        ("h,help", "Print usage");

    options.allow_unrecognised_options();
    auto result = options.parse(argc, argv);

    if (result.count("help") > 0) {
        std::cout << options.help() << std::endl;
        std::exit(0);
    }

    auto num_threads = result["threads"].as<uint32_t>();
    auto num_runs = result["runs"].as<uint32_t>();
    auto dataset_a_path = result["dataseta"].as<std::string>();
    auto dataset_b_path = result["datasetb"].as<std::string>();

    auto dataset_a = HashJoinBenchmark::load_dataset_a(dataset_a_path);
    auto dataset_b = HashJoinBenchmark::load_dataset_b(dataset_b_path);

    auto benchmark_impl_name = result["implementation"].as<std::string>();

    std::cout << "Num threads: " << num_threads << std::endl;
    std::cout << "Num runs: " << num_runs << std::endl;
    std::cout << "Num smaller: " << dataset_a.size() << std::endl;
    std::cout << "Num larger:  " << dataset_b.size() << std::endl;

    if (benchmark_impl_name == "libcuckoo") {
        std::cout << "Benchmarking libcuckoo!" << std::endl;
        return HashJoinBenchmark::run_benchmark<HashJoinBenchmark::CuckooMap>("libcuckoo", dataset_a, dataset_b, num_runs, num_threads);
    } else if (benchmark_impl_name == "tbb-unordered") {
        std::cout << "Benchmarking TBB concurrent_unordered_map!" << std::endl;
        return HashJoinBenchmark::run_benchmark<HashJoinBenchmark::TBBUnorderedMap>("tbb-unordered", dataset_a, dataset_b, num_runs, num_threads);
    } else if (benchmark_impl_name == "tbb-hash") {
        std::cout << "Benchmarking TBB concurrent_hash_map!" << std::endl;
        return HashJoinBenchmark::run_benchmark<HashJoinBenchmark::TBBHashMap>("tbb-hash", dataset_a, dataset_b, num_runs, num_threads);
    } else if (benchmark_impl_name == "std-blocking") {
        std::cout << "Benchmarking Blocking STD!" << std::endl;
        return HashJoinBenchmark::run_benchmark<HashJoinBenchmark::STDMap>("std-blocking", dataset_a, dataset_b, num_runs, num_threads);
    } else if (benchmark_impl_name == "junction-grampa") {
        std::cout << "Benchmarking Junction ConcurrentMap_Grampa!" << std::endl;
        return HashJoinBenchmark::run_benchmark<HashJoinBenchmark::JunctionMapGrampa>("junction-grampa", dataset_a, dataset_b, num_runs, num_threads);
    } else if (benchmark_impl_name == "junction-leapfrog") {
        std::cout << "Benchmarking Junction ConcurrentMap_Leapfrog!" << std::endl;
        return HashJoinBenchmark::run_benchmark<HashJoinBenchmark::JunctionMapLeapfrog>("junction-leapfrog", dataset_a, dataset_b, num_runs, num_threads);
    } else {
        std::cerr << "Unknown implementation " << benchmark_impl_name << std::endl;
        std::exit(-1);
    }
}

auto main_cache(int argc, const char** argv) -> BenchmarkResult {
    cxxopts::Options options("HashmapBenchmark hashjoin", "Benchmark multiple concurrent hashmaps (Cache benchmark)!");

    options.add_options()
        ("t,threads", "Number of threads", cxxopts::value<uint32_t>()->default_value("16"))
        ("r,runs", "Number of runs per hashmap", cxxopts::value<uint32_t>()->default_value("10"))
        ("j,json", "Path to JSON output", cxxopts::value<std::string>()->implicit_value("json_out.json"))
        ("i,implementation", "Map implementation to use", cxxopts::value<std::string>()->implicit_value("std-blocking"))
        ("s,seed", "Random seed to use", cxxopts::value<uint64_t>()->default_value("37"))
        ("l,limit", "Time limit for this benchmark (ms)", cxxopts::value<uint64_t>()->default_value("30000"))
        ("c,capacity", "Map capacity (affects number of max indices)", cxxopts::value<uint64_t>()->default_value("500000"))
        ("h,help", "Print usage");

    options.allow_unrecognised_options();
    auto result = options.parse(argc, argv);

    if (result.count("help") > 0) {
        std::cout << options.help() << std::endl;
        std::exit(0);
    }

    auto num_threads = result["threads"].as<uint32_t>();
    auto num_runs = result["runs"].as<uint32_t>();
    auto seed = result["seed"].as<uint64_t>();
    auto time_limit = result["limit"].as<uint64_t>();
    auto capacity = result["capacity"].as<uint64_t>();

    auto benchmark_impl_name = result["implementation"].as<std::string>();

    if (benchmark_impl_name == "libcuckoo") {
        std::cout << "Benchmarking libcuckoo!" << std::endl;
        return CacheBenchmark::run_benchmark<CacheBenchmark::CuckooMap>("libcuckoo", seed, time_limit, capacity, num_runs, num_threads);
    } else if (benchmark_impl_name == "tbb-hash") {
        std::cout << "Benchmarking TBB concurrent_hash_map!" << std::endl;
        return CacheBenchmark::run_benchmark<CacheBenchmark::TBBHashMap>("tbb-hash", seed, time_limit, capacity, num_runs, num_threads);
    } else if (benchmark_impl_name == "std-blocking") {
        std::cout << "Benchmarking Blocking STD!" << std::endl;
        return CacheBenchmark::run_benchmark<CacheBenchmark::STDMap>("std-blocking", seed, time_limit, capacity, num_runs, num_threads);
    } else if (benchmark_impl_name == "junction-grampa") {
        std::cout << "Benchmarking Junction ConcurrentMap_Grampa!" << std::endl;
        return CacheBenchmark::run_benchmark<CacheBenchmark::JunctionMapGrampa>("junction-grampa", seed, time_limit, capacity, num_runs, num_threads);
    } else if (benchmark_impl_name == "junction-leapfrog") {
        std::cout << "Benchmarking Junction ConcurrentMap_Leapfrog!" << std::endl;
        return CacheBenchmark::run_benchmark<CacheBenchmark::JunctionMapLeapfrog>("junction-leapfrog", seed, time_limit, capacity, num_runs, num_threads);
    } else {
        std::cerr << "Unknown implementation " << benchmark_impl_name << std::endl;
        std::exit(-1);
    }
}

auto main(int argc, const char** argv) -> int {
    cxxopts::Options options("HashmapBenchmark", "Benchmark multiple concurrent hashmaps!");
    options.add_options()
        ("benchmark", "", cxxopts::value<std::string>())
        ("h,help", "Print usage")
        ("j,json", "Path to JSON output", cxxopts::value<std::string>()->implicit_value("json_out.json"));

    options.positional_help("[benchmark]");
    options.parse_positional({"benchmark"});
    options.allow_unrecognised_options();

    auto result = options.parse(argc, argv);

    if (result.count("help") > 0 && result.count("benchmark") == 0) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    BenchmarkResult benchmark_result;

    if (result.count("benchmark") > 0) {
        auto benchmark = result["benchmark"].as<std::string>();

        if (benchmark == "wordcount") {
            benchmark_result = main_wordcount(argc, argv);
        } else if (benchmark == "hashjoin") {
            benchmark_result = main_hashjoin(argc, argv);
        } else if (benchmark == "cache") {
            benchmark_result = main_cache(argc, argv);
        } else {
            std::cout << "Unknown benchmark " << benchmark << std::endl;
            std::cout << options.help() << std::endl;
            return -1;
        }
    } else {
        std::cout << "No benchmark selected!" << std::endl;
        std::cout << options.help() << std::endl;
        return -1;
    }

    std::cout << "Benchmark result:" << std::endl;
    std::cout << "Impl:      " << benchmark_result.impl << std::endl;
    std::cout << "Correct:   " << benchmark_result.correct << std::endl;
    std::cout << "Hash:      " << benchmark_result.hash << std::endl;
    std::cout << "Runtime:   " << benchmark_result.total_value << std::endl;
    std::cout << "Min value:  " << benchmark_result.min_value << std::endl;
    std::cout << "Max value:  " << benchmark_result.max_value << std::endl;
    std::cout << "Avg value:  " << benchmark_result.avg_value << std::endl;
    std::cout << "Mean value: " << benchmark_result.mean_value << std::endl;

    std::cout << "json?: " << result.count("json") << std::endl;

    if (result.count("json")) {
        write_file(result["json"].as<std::string>(), JSONSerializer::serialize_benchmark_result(benchmark_result));
    }
    
    return 0;
}