#pragma once
#include "hashjoin.hpp"
#include <unordered_map>
#include <mutex>

namespace HashJoinBenchmark {
    class STDMap {
        public:
            auto insert(uint32_t key, const DatasetAValue& value) -> void {
                std::lock_guard<std::mutex> guard(this->mtx);
                this->map.insert({key, value});
            }

            auto get(uint32_t key) -> const DatasetAValue& {
                return this->map.at(key);
            }

        private:
            std::unordered_map<uint32_t, DatasetAValue> map;
            std::mutex mtx;
    };
}