#include <cstdint>
#include <libcuckoo/cuckoohash_map.hh>
#include <junction/ConcurrentMap_Grampa.h>
#include <tbb/concurrent_unordered_map.h>
#include <cxxopts.hpp>
#include <string>

auto main(int argc, const char** argv) -> int {
    cxxopts::Options options("HashmapBenchmark", "Benchmark multiple concurrent hashmaps!");

    options.add_options()
        ("t,threads", "Number of threads", cxxopts::value<uint32_t>()->default_value("16"))
        ("r,runs", "Number of runs per hashmap", cxxopts::value<uint32_t>()->default_value("10"))
        ("d,dataset", "Path to the used dataset", cxxopts::value<std::string>()->default_value("../data/train.ft.txt.out"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
    }

    auto num_threads = result["threads"].as<uint32_t>();
    auto num_runs = result["runs"].as<uint32_t>();
    auto dataset_path = result["dataset"].as<std::string>();

    tbb::concurrent_unordered_map<std::string, std::string> tbb_map;
    libcuckoo::cuckoohash_map<std::string, std::string> libcuckoo_map;
    junction::ConcurrentMap_Grampa<turf::u64, turf::u64> junction_map;

    tbb_map.insert({"hello", "world"});
    libcuckoo_map.insert("hello", "world");
    junction_map.assign(1, 2);

    std::cout << "Threads: " << num_threads << ", runs: " << num_runs << std::endl;
    std::cout << "Path to dataset: " << dataset_path << std::endl;
    return 0;
}