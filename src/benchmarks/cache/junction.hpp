#pragma once
#include "cache.hpp"
#include <junction/ConcurrentMap_Grampa.h>
#include <junction/ConcurrentMap_Leapfrog.h>
#include <junction/ConcurrentMap_Crude.h>
#include <junction/ConcurrentMap_Linear.h>

namespace CacheBenchmark {
    template<typename MapType>
    class JunctionMap {
        public:
            JunctionMap(uint64_t capacity) : capacity(capacity), size(0), map(capacity) {
            }

            auto access(uint64_t key) -> CacheData {
                // Junction needs the key 0 for it's own purposes
                // so we modify tke key by 1
                auto mutator = this->map.insertOrFind(key + 1);
                auto value = mutator.getValue();

                // If we just inserted a key, it will not be initialized
                // Junction needs the values 0 (default) & 1 (redirect?) reserved for it's own
                // purposes, so we modify the values by 2
                if (value != (key + 2)) {
                    mutator.assignValue(key + 2);
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