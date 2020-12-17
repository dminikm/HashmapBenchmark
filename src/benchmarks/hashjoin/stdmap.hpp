#pragma once
#include "hashjoin.hpp"
#include <unordered_map>
#include <mutex>

namespace HashJoinBenchmark {
    class STDMap {
        public:
            STDMap() {

            }

            auto insert(uint32_t key, DatasetAValue value) -> void {
                std::lock_guard<std::mutex> guard(this->mtx);
                this->map.insert({key, value});
            }

            auto get(uint32_t key) -> DatasetAValue {
                //std::lock_guard<std::mutex> guard(this->mtx);
                return this->map.at(key);
            }

        private:
            std::unordered_map<uint32_t, DatasetAValue> map;
            std::mutex mtx;
    };
}