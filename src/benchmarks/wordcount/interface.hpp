#pragma once
#include <cstdint>
#include <string_view>

namespace WordCountBenchmark {
    class WordCountMapInterface {
        public:
            using KeyValues = std::vector<std::pair<std::string_view, uint32_t>>;
            
            virtual void increase_or_insert(std::string_view key, uint64_t def) = 0;
            virtual KeyValues get_key_value_pairs() = 0;
    };
}