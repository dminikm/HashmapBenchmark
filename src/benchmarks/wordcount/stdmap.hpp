#pragma once
#include <mutex>
#include "wordcount.hpp"

namespace WordCountBenchmark {
    class AtomicSTDMap : public WordCountMapInterface {
        public:
            virtual void increase_or_insert(std::string_view key, uint64_t def) {
                this->map[key].fetch_add(1, std::memory_order::memory_order_acq_rel);
            }

            virtual KeyValues get_key_value_pairs() {
                KeyValues kvs;
                kvs.reserve(this->map.size());

                for (auto& [key, value] : this->map) {
                    kvs.push_back(std::make_pair(key, value.load()));
                }

                std::stable_sort(kvs.begin(), kvs.end());

                return kvs;
            }

        private:
            std::unordered_map<std::string_view, std::atomic<uint32_t>> map;
    };

    class BlockingSTDMap : public WordCountMapInterface {
        public:
            virtual void increase_or_insert(std::string_view key, uint64_t def) {
                std::lock_guard<std::mutex> guard(this->mtx);
                this->map[key] += 1;
            }

            virtual KeyValues get_key_value_pairs() {
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