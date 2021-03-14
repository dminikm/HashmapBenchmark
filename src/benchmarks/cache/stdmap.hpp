#pragma once
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <atomic>
#include "cache.hpp"

namespace CacheBenchmark {
    class STDMap {
        public:
            STDMap(uint64_t capacity) : capacity(capacity), size(0) {
                this->map.reserve(capacity);
            }

            auto access(uint64_t key) -> CacheData {
                // Shared lock scope
                {
                    std::shared_lock lock(this->mtx);
                    auto res = this->map.find(key);
                    if (res != this->map.end()) {
                        return res->second;
                    }
                }
                
                {
                    auto size = this->get_size();
                    auto capacity = this->get_capacity();

                    // Wait while we have less than 2% of free space
                    while (size > capacity - (capacity / 50)) {
                        size = this->get_size();
                    }

                    // No value found, bring out the exclusive lock
                    std::unique_lock lock(this->mtx);
                    auto result = this->map.emplace(
                        key,
                        key
                    );

                    if (result.second)
                        this->size.fetch_add(1);

                    return result.first->second;
                }
            }

            auto erase(uint64_t key) -> void {
                std::unique_lock lock(this->mtx);
                if (this->map.erase(key) > 0)
                    this->size.fetch_sub(1);
            }

            auto get_size() const -> uint64_t {
                return this->size.load();
            }

            auto get_capacity() const -> uint64_t {
                return this->capacity;
            }

        private:
            std::shared_mutex mtx{};
            std::unordered_map<uint64_t, CacheData> map{};

            std::atomic<uint64_t> size;
            uint64_t capacity;
    };
}