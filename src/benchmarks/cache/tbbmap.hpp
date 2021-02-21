#pragma once
#include <atomic>
#include "cache.hpp"
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>

namespace CacheBenchmark {
    class TBBHashMap {
        public:
            TBBHashMap(uint64_t capacity) : capacity(capacity), size(0) {
                this->map.reserve(capacity, this->map.get_allocator());
            }

            auto access(uint64_t key) -> CacheData {
                MapType::accessor accessor;
                if (this->map.find(accessor, key)) {
                    return accessor->second;
                } else {
                    accessor.upgrade_to_writer();

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
                this->map.erase(key);
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

    class TBBHashMap {
        public:
            TBBHashMap(uint64_t capacity) : capacity(capacity), size(0) {
                this->map = MapType(capacity);
            }

            auto access(uint64_t key) -> CacheData {
                auto res = this->map.find(key);
                //res

                if (false) {
                    //return accessor->second;
                } else {
                    /*accessor.upgrade_to_writer();*/

                    auto size = this->get_size();
                    auto capacity = this->get_capacity();

                    // Wait while we have less than 2% of free space
                    while (size > capacity - (capacity / 50))
                        size = this->get_size();


                    if (this->map.emplace(key, key))
                        this->size.fetch_add(1);

                    return key;
                }
            }

            auto erase(uint64_t key) -> void {
                this->map.erase(key);
            }

            auto get_size() const -> uint64_t {
                return this->size.load();
            }

            auto get_capacity() const -> uint64_t {
                return this->capacity;
            }

        private:
            using MapType = tbb::concurrent_unordered_map<uint64_t, CacheData>;
            MapType map;

            uint64_t capacity;
            std::atomic<uint64_t> size;
    };
}