#include <cstdint>
#include <libcuckoo/cuckoohash_map.hh>
#include <junction/ConcurrentMap_Grampa.h>
#include <tbb/concurrent_unordered_map.h>
#include <cxxopts.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cctype>

using WordFile = std::vector<std::string>;
using CuckooMap = libcuckoo::cuckoohash_map<std::string_view, uint32_t>;

class Semaphore {
    public:
        auto wait() -> void {
            std::unique_lock<std::mutex> lock(this->mut);
            var.wait(lock, [this]() { return this->ready; });
        }

        auto notify_all() -> void {
            std::lock_guard<std::mutex> lock(this->mut);
            this->ready = true;
            this->var.notify_all();
        }

    private:
        std::mutex mut;
        std::condition_variable var{};
        bool ready = false;
};


auto load_file(const std::string& path) -> std::optional<WordFile> {
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

auto libcuckoo_count_words_impl(Semaphore& semaphore, const WordFile& file, CuckooMap& map, uint32_t start, uint32_t end) -> void {
    // Wait for test start
    semaphore.wait();

    for (auto i = start; i < end; i++) {
        auto& line = file[i];

        uint32_t word_start = 0;
        for (auto o = 0; o < line.size(); o++) {
            auto ch = line[o];

            if (!std::isalnum(ch)) {
                uint32_t len = o - word_start;

                // Empty word
                if (len == 0) {
                    word_start = o;
                    break;
                }

                map.upsert(std::string_view(line.c_str() + word_start, len), [](uint32_t& value) -> bool {
                    value += 1;
                    return false;
                }, 1);

                word_start = o + 1;
            }
        }
    }
}

auto libcuckoo_count_words(const WordFile& file, uint32_t num_threads) -> CuckooMap {
    CuckooMap map;
    Semaphore sem;
    
    if (num_threads == 1) {
        sem.notify_all();
        // Insert timing here
        libcuckoo_count_words_impl(sem, file, map, 0, file.size());
        // End timing here
    } else {
        auto even_split = file.size() / num_threads;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (auto i = 0; i < num_threads; i++) {
            auto start = i * even_split;
            auto end = (i == num_threads - 1) ? file.size() : ((i + 1) * even_split);

            threads.push_back(
                std::thread(
                    &libcuckoo_count_words_impl,
                    std::ref(sem),
                    std::cref(file),
                    std::ref(map),
                    start,
                    end
                )
            );
        }

        sem.notify_all();
        // Insert timing here

        // End of timing
        // Join all threads
        for (auto& th : threads) {
            th.join();
        }
    }

    return map;
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
    }

    auto num_threads = result["threads"].as<uint32_t>();
    auto num_runs = result["runs"].as<uint32_t>();
    auto dataset_path = result["dataset"].as<std::string>();

    auto file = load_file(dataset_path);

    if (!file) {
        std::cout << "Dataset '" << dataset_path << "' does not exist, aborting!" << std::endl;
    }

    auto map = libcuckoo_count_words(*file, num_threads);
    std::ofstream out_file("libcuckoo_result.txt");
    
    for (auto& [key, value] : map.lock_table()) {
        out_file << key << " : " << value << std::endl;
    }

    return 0;
}