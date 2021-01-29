#pragma once
#include "hashjoin.hpp"
#include <libcuckoo/cuckoohash_map.hh>

namespace HashJoinBenchmark {
    class CuckooMap {
        public:
            auto insert(uint32_t key, const DatasetAValue& value) -> void {
                this->map.insert(key, value);
            }

            auto get(uint32_t key) -> DatasetAValue {
                return this->map.find(key);
            }

        private:
            libcuckoo::cuckoohash_map<uint32_t, DatasetAValue> map;
    };
}