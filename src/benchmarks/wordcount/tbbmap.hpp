#pragma once
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>
#include "wordcount.hpp"

namespace WordCountBenchmark {
    class TBBUnorderedMap : public WordCountMapInterface {
        public:
            virtual void increase_or_insert(std::string_view key, uint64_t def) {
                this->map[key].fetch_and_increment();
            }

            virtual KeyValues get_key_value_pairs() {
                KeyValues kvs;
                kvs.reserve(this->map.size());

                for (auto& [key, value] : this->map) {
                    kvs.push_back(std::make_pair(key, value));
                }

                std::stable_sort(kvs.begin(), kvs.end());

                return kvs;
            }

        private:
            tbb::concurrent_unordered_map<std::string_view, tbb::atomic<uint32_t>, std::hash<std::string_view>> map;
    };

    class TBBHashMap : public WordCountMapInterface {
        private:
            class StringViewHashCompare {
                public:
                    auto hash(const std::string_view& str) const -> size_t {
                        return std::hash<std::string_view>{}(str);
                    }

                    auto equal(const std::string_view& a, const std::string_view& b) const -> bool {
                        return a == b;
                    }
            };

        public:
            virtual void increase_or_insert(std::string_view key, uint64_t def) {
                MapType::accessor ac;
                if (map.find(ac, key)) {
                    ac->second += 1;
                    ac.release();
                } else {
                    map.insert(ac, std::make_pair(key, def));
                }
            }

            virtual KeyValues get_key_value_pairs() {
                KeyValues kvs;
                kvs.reserve(this->map.size());

                for (auto& [key, value] : this->map) {
                    kvs.push_back(std::make_pair(key, value));
                }

                std::stable_sort(kvs.begin(), kvs.end());

                return kvs;
            }

        private:
            using MapType = tbb::concurrent_hash_map<std::string_view, uint32_t, StringViewHashCompare>;
            MapType map;
    };
}