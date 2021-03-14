#pragma once
#include "cache.hpp"
#include <junction/ConcurrentMap_Grampa.h>
#include <junction/ConcurrentMap_Leapfrog.h>
#include <junction/ConcurrentMap_Crude.h>
#include <junction/ConcurrentMap_Linear.h>

namespace CacheBenchmark {
    auto nearest_power_of_2(uint64_t n) -> uint64_t {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        n++;

        return n;
    }

    template<typename MapType>
    class JunctionMap {
        public:
            // It's never mentioned anywhere, but leapfrogs size needs to be a power of 2
            JunctionMap(uint64_t capacity) : capacity(capacity), size(0), map(nearest_power_of_2(capacity)) {
            }

            auto access(uint64_t key) -> CacheData {
                // Junction needs the key 0 for it's own purposes
                // so we modify tke key by 1

                auto mutator = this->map.find(key + 1);
                auto value = mutator.getValue();

                if (value == 0) {
                    auto size = this->get_size();
                    auto capacity = this->get_capacity();

                    // Wait while we have less than 2% of free space
                    while (size > capacity - (capacity / 50)) {
                        size = this->get_size();
                    }

                    auto mutator = this->map.insertOrFind(key + 1);

                    // Junction needs the values 0 (Default) and 1(Redirect) for it's own purposes
                    // so we modify the value by 2
                    auto old_value = mutator.exchangeValue(value + 2);
                    if (old_value == 0)
                        this->size.fetch_add(1);

                    return key;
                } else {
                    return value - 2;
                }
            }

            auto erase(uint64_t key) -> void {
                if (this->map.erase(key + 1) == 0)
                    this->size.fetch_sub(1);
            }

            auto get_size() const -> uint64_t {
                return this->size.load();
            }

            auto get_capacity() const -> uint64_t {
                return this->capacity;
            }

        private:
            MapType map;
            uint64_t capacity;
            std::atomic<uint64_t> size;
    };

    using JunctionMapGrampa = JunctionMap<junction::ConcurrentMap_Grampa<uint64_t, CacheData>>;
    using JunctionMapLeapfrog = JunctionMap<junction::ConcurrentMap_Leapfrog<uint64_t, CacheData>>;
}