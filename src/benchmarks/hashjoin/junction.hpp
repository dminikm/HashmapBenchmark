#pragma once
#include "hashjoin.hpp"
#include <junction/ConcurrentMap_Grampa.h>
#include <junction/ConcurrentMap_Leapfrog.h>
#include <junction/ConcurrentMap_Crude.h>
#include <junction/ConcurrentMap_Linear.h>

namespace HashJoinBenchmark {

    template <typename MapType, typename IteratorType>
    class JunctionMap {
        public:
            ~JunctionMap() {
                IteratorType iter(this->map);

                while (iter.isValid()) {
                    // Delete held value
                    delete iter.getValue();

                    // Move to the next element
                    iter.next();
                }
            }

            auto insert(uint32_t key, const DatasetAValue& value) -> void {
                auto heapValue = new DatasetAValue(value);
                this->map.assign(key, heapValue);
            }

            auto get(uint32_t key) -> const DatasetAValue& {
                return *this->map.get(key);
            }

        private:
            MapType map;
    };

    using JunctionMapGrampaInner = junction::ConcurrentMap_Grampa<turf::u32, DatasetAValue*>;
    using JunctionMapGrampa = JunctionMap<JunctionMapGrampaInner, JunctionMapGrampaInner::Iterator>;

    using JunctionMapLeapfrogInner = junction::ConcurrentMap_Leapfrog<turf::u32, DatasetAValue*>;
    using JunctionMapLeapfrog = JunctionMap<JunctionMapLeapfrogInner, JunctionMapLeapfrogInner::Iterator>;
}