#pragma once
#include <cstdint>
#include <string_view>

namespace WordCountBenchmark {
    class WordCountMapInterface {
        public:
            using KeyValues = std::vector<std::pair<std::string_view, uint32_t>>;
    };
}