#pragma once
#include <tbb/concurrent_unordered_map.h>
#include "wordcount.hpp"

namespace WordCountBenchmark {
    using TBBMap = tbb::concurrent_unordered_map<std::string_view, tbb::atomic<uint32_t>, std::hash<std::string_view>>;

    inline auto hash_tbb_map(TBBMap& map) -> uint64_t {
        std::vector<std::string_view> keys;
        keys.reserve(map.size());

        for (auto& [key, value] : map) {
            keys.push_back(key);
        }

        std::stable_sort(keys.begin(), keys.end());

        uint64_t hash = keys.size();

        for (auto& key : keys) {
            auto key_hash = std::hash<std::string_view>{}(key);
            auto value_hash = std::hash<uint32_t>{}(map.at(key));

            hash ^= key_hash + value_hash + 0x9E3779B9 + (hash << 6) + (hash >> 2);
        }

        return hash;
    }

    inline auto tbbmap_count_words_impl(Semaphore& semaphore, const WordFile& file, TBBMap& map, uint32_t start, uint32_t end) -> void {
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

                    map[std::string_view(line.c_str() + word_start, len)].fetch_and_add(1);

                    word_start = o + 1;
                }
            }
        }
    }

    inline auto tbbmap_count_words(const WordFile& file, uint32_t num_threads) -> RunResult {
        TBBMap map;
        Semaphore sem;
        
        RunResult result;
        auto even_split = file.size() / num_threads;
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (auto i = 0; i < num_threads; i++) {
            auto start = i * even_split;
            auto end = (i == num_threads - 1) ? file.size() : ((i + 1) * even_split);

            threads.push_back(
                std::thread(
                    &tbbmap_count_words_impl,
                    std::ref(sem),
                    std::cref(file),
                    std::ref(map),
                    start,
                    end
                )
            );
        }

        // Start timer
        Timer t;
        t.start();

        // Wake up threads
        sem.notify_all();

        // Join all threads
        for (auto& th : threads) {
            th.join();
        }

        // End timer
        t.end();

        result.hash = hash_tbb_map(map);
        result.time = t.get_duration();

        return result;
    }
}