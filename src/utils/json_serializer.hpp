#pragma once
#include <string>
#include <sstream>

#include "../benchmarks/benchmark.hpp"

class JSONSerializer {
    public:
        static auto serialize_run_results(BenchmarkResult& result) -> std::string {
            std::stringstream ss;

            ss << "[\n";

            for (int i = 0; i < result.runs.size(); i++) {
                auto& run = result.runs[i];

                if (i) {
                    ss << ",\n";
                }

                ss << "        " << "{\n";
                ss << "            " << "\"value\": " << run.value << ",\n";
                ss << "            " << "\"hash\": " << run.hash << "\n";
                ss << "        " << "}";
            }

            ss << "\n    ],";
            return ss.str();
        }

        static auto serialize_benchmark_result(BenchmarkResult& result) -> std::string {
            std::stringstream ss;

            ss << "{\n";

            ss << "    " << "\"runs\": ";

            ss << JSONSerializer::serialize_run_results(result) << "\n";

            ss << "    " << "\"implementation\": "  << "\"" << result.impl << "\"" << ",\n";
            ss << "    " << "\"value_unit\": "      << "\"" << result.value_unit << "\"" << ",\n";
            ss << "    " << "\"correct\": "         << (result.correct ? "true" : "false") << ",\n";
            ss << "    " << "\"num_runs\": "        << result.num_runs << ",\n";
            ss << "    " << "\"num_threads\": "     << result.num_threads << ",\n";
            ss << "    " << "\"total_value\": "     << result.total_value << ",\n";
            ss << "    " << "\"min_value\": "       << result.min_value << ",\n";
            ss << "    " << "\"avg_value\": "       << result.avg_value << ",\n";
            ss << "    " << "\"mean_value\": "      << result.mean_value << ",\n";
            ss << "    " << "\"max_value\": "       << result.max_value << "\n";

            ss << "}\n";

            return ss.str();
        }
};