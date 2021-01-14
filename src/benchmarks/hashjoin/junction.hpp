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

            /*auto get_thread_qsrb_context() -> junction::QSBR::Context {
                static thread_local auto context = junction::defaultQSRB.createContext();
                return context;
            }*/

            auto insert(uint32_t key, DatasetAValue value) -> void {
                /*auto context = get_thread_qsrb_context();

                junction::DefaultQSBR.update(context);*/
                auto valueHeap = new DatasetAValue(value);
                this->map.assign(key, valueHeap);
            }

            auto get(uint32_t key) -> DatasetAValue {
                /*auto context = get_thread_qsrb_context();

                junction::DefaultQSBR.update(context);*/
                return *this->map.get(key);
            }

        private:
            MapType map;
    };

    using JunctionMapGrampaInner = junction::ConcurrentMap_Grampa<turf::u32, DatasetAValue*>;
    using JunctionMapGrampa = JunctionMap<JunctionMapGrampaInner, JunctionMapGrampaInner::Iterator>;

    using JunctionMapLeapfrogInner = junction::ConcurrentMap_Leapfrog<turf::u32, DatasetAValue*>;
    using JunctionMapLeapfrog = JunctionMap<JunctionMapLeapfrogInner, JunctionMapLeapfrogInner::Iterator>;

    /*using JunctionMapLeapfrog = JunctionMap<junction::ConcurrentMap_Leapfrog<turf::u32, DatasetAValue*>>;
    using JunctionMapCrude = JunctionMap<junction::ConcurrentMap_Crude<turf::u32, DatasetAValue*>>;
    using JunctionMapLinear = JunctionMap<junction::ConcurrentMap_Linear<turf::u32, DatasetAValue*>>;*/
}