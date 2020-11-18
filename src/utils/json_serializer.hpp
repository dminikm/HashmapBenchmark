#pragma once
#include <string>
#include <sstream>

#include "../benchmarks/wordcount/wordcount.hpp"

class JSONSerializer {
    public:
        static auto serialize_run_results(WordCountBenchmark::BenchmarkResult& result) -> std::string {
            std::stringstream ss;

            ss << "[\n";

            for (int i = 0; i < result.runs.size(); i++) {
                auto& run = result.runs[i];

                if (i) {
                    ss << ",\n";
                }

                ss << "        " << "{\n";
                ss << "            " << "\"time\": " << run.time << ",\n";
                ss << "            " << "\"hash\": " << run.hash << "\n";
                ss << "        " << "}";
            }

            ss << "\n    ],";
            return ss.str();
        }

        static auto serialize_benchmark_result(WordCountBenchmark::BenchmarkResult& result) -> std::string {
            std::stringstream ss;

            ss << "{\n";

            ss << "    " << "\"runs\": ";

            ss << JSONSerializer::serialize_run_results(result) << "\n";

            ss << "    " << "\"implementation\": " << "\"" << result.impl << "\"" << ",\n";
            ss << "    " << "\"correct\": "        << (result.correct ? "true" : "false") << ",\n";
            ss << "    " << "\"num_runs\": "       << result.num_runs << ",\n";
            ss << "    " << "\"num_threads\": "    << result.num_threads << ",\n";
            ss << "    " << "\"total_time\": "     << result.total_time << ",\n";
            ss << "    " << "\"min_time\": "       << result.min_time << ",\n";
            ss << "    " << "\"avg_time\": "       << result.avg_time << ",\n";
            ss << "    " << "\"mean_time\": "      << result.mean_time << ",\n";
            ss << "    " << "\"max_time\": "       << result.max_time << "\n";

            ss << "}\n";

            return ss.str();
        }
};