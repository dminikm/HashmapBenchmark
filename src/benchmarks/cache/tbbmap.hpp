#pragma once
#include <atomic>
#include "cache.hpp"
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>

namespace CacheBenchmark {
    class TBBHashMap {
        public:
            TBBHashMap(uint64_t capacity) : capacity(capacity), size(0) {
                // We can't use reserve on concurrent_hash_map as it's inherited as protected
            }

            auto access(uint64_t key) -> CacheData {
                MapType::accessor accessor;
                if (this->map.find(accessor, key)) {
                    return accessor->second;
                } else {
                    auto size = this->get_size();
                    auto capacity = this->get_capacity();

                    // Wait while we have less than 2% of free space
                    while (size > capacity - (capacity / 50))
                        size = this->get_size();

                    if (map.emplace(accessor, key, key))
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
            using MapType = tbb::concurrent_hash_map<uint64_t, CacheData>;
            MapType map;

            uint64_t capacity;
            std::atomic<uint64_t> size;
    };
}