#pragma once
#include <atomic>
#include "cache.hpp"
#include <libcuckoo/cuckoohash_map.hh>

namespace CacheBenchmark {
    class CuckooMap {
        public:
            CuckooMap(uint64_t capacity) : capacity(capacity), size(0) {
                this->map.reserve(capacity);
            }

            auto access(uint64_t key) -> CacheData {
                CacheData result{};

                if (this->map.find(key, result)) {
                    return result;
                } else {
                    auto size = this->get_size();
                    auto capacity = this->get_capacity();

                    // Wait while we have less than 2% of free space
                    while (size > capacity - (capacity / 50)) {
                        size = this->get_size();
                    }

                    if (this->map.insert(key, key))
                        this->size.fetch_add(1);
                    
                    return key;
                }
            }

            auto erase(uint64_t key) -> void {
                if (this->map.erase(key))
                    this->size.fetch_sub(1);
            }

            auto get_size() const -> uint64_t {
                return this->size.load();
            }

            auto get_capacity() const -> uint64_t {
                return this->capacity;
            }

        private:
            libcuckoo::cuckoohash_map<uint64_t, CacheData> map;

            uint64_t capacity;
            std::atomic<uint64_t> size;
    };
}