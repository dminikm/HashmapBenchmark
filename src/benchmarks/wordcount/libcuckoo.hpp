#pragma once
#include <libcuckoo/cuckoohash_map.hh>
#include "interface.hpp"
#include "wordcount.hpp"

namespace WordCountBenchmark {
    class CuckooMap : public WordCountMapInterface {
        public:
            virtual void increase_or_insert(std::string_view key, uint64_t def) {
                this->map.upsert(key, [](uint32_t& value) -> bool {
                    value += 1;
                    return false;
                }, def);
            }

            virtual KeyValues get_key_value_pairs() {
                KeyValues kvs;
                kvs.reserve(this->map.size());

                auto lt = map.lock_table();
                for (auto& [key, value] : lt) {
                    kvs.push_back(std::make_pair(key, value));
                }

                std::stable_sort(kvs.begin(), kvs.end());

                return kvs;
            }

        private:
            libcuckoo::cuckoohash_map<std::string_view, uint32_t> map;
    };
}