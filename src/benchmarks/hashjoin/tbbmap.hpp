#pragma once
#include "hashjoin.hpp"
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>

namespace HashJoinBenchmark {
    class TBBHashMap {
        public:
            auto insert(uint32_t key, DatasetAValue value) -> void {
                this->map.insert({key, value});
            }

            auto get(uint32_t key) -> DatasetAValue {
                MapType::accessor accessor;
                this->map.find(accessor, key);
                auto value_copy = accessor->second;
                accessor.release();

                return value_copy;
            }

        private:
            using MapType = tbb::concurrent_hash_map<uint32_t, DatasetAValue>;
            MapType map;
    };

    class TBBUnorderedMap {
        public:
            auto insert(uint32_t key, DatasetAValue value) -> void {
                this->map.insert({key, value});
            }

            auto get(uint32_t key) -> DatasetAValue {
                auto result = this->map.find(key);
                return result->second;
            }

        private:
            tbb::concurrent_unordered_map<uint32_t, DatasetAValue> map;
    };
}