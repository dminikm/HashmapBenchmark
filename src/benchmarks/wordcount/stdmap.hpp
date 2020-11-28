#pragma once
#include <mutex>
#include "wordcount.hpp"

namespace WordCountBenchmark {
    class BlockingSTDMap : public WordCountMapInterface {
        public:
            inline void increase_or_insert(std::string_view key, uint64_t def) {
                std::lock_guard<std::mutex> guard(this->mtx);
                this->map[key] += 1;
            }

            inline KeyValues get_key_value_pairs() {
                std::lock_guard<std::mutex> guard(this->mtx);

                KeyValues kvs;
                kvs.reserve(this->map.size());

                for (auto& [key, value] : this->map) {
                    kvs.push_back(std::make_pair(key, value));
                }

                std::stable_sort(kvs.begin(), kvs.end());

                return kvs;
            }

        private:
            std::unordered_map<std::string_view, uint32_t> map;
            std::mutex mtx;
    };
}