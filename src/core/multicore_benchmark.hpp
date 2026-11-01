#pragma once

#include "fea.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace workbench {

struct BatchConfig {
    int case_count = 500;
    int mesh_size = 100;
    uint32_t seed = 42;
    double k_min = 1.0;
    double k_max = 50.0;
    double q_min = 1000.0;
    double q_max = 50000.0;
};

struct SystemInfo {
    unsigned int logical_cpus = 1;
    std::string compiler_name;
    std::string compiler_version;
    std::string build_config;
    std::string optimization_level;
    bool speedup_assessable = true;
};

struct MulticoreWorkerResult {
    int worker_count = 1;
    std::vector<double> repetition_times_ms; // 5 measured repetitions
    double median_time_ms = 0.0;
    double speedup = 1.0;                    // T1 / Tp
    double efficiency = 1.0;                 // speedup / p
};

struct MulticoreBenchmarkResult {
    BatchConfig config;
    SystemInfo system;
    double max_abs_diff_vs_serial = 0.0;
    bool verification_passed = false;        // max abs diff <= 1e-8
    std::vector<MulticoreWorkerResult> runs;
};

// Get current host system compilation and CPU info
SystemInfo get_system_info();

// Generate deterministic batch of FEA cases using std::mt19937 with fixed seed
std::vector<Fea1DParameters> generate_seeded_batch(const BatchConfig& config);

// Serial execution of batch
std::vector<Fea1DSolution> solve_batch_serial(const std::vector<Fea1DParameters>& batch);

// Parallel execution using ThreadPool with results strictly stored in input order
std::vector<Fea1DSolution> solve_batch_parallel(const std::vector<Fea1DParameters>& batch, int num_workers);

// Compare two solution batches and find maximum absolute temperature difference
double compare_solution_batches(
    const std::vector<Fea1DSolution>& sol_a,
    const std::vector<Fea1DSolution>& sol_b
);

// Full benchmark suite: 1 warmup, 5 measured reps, calculates median, speedup, and efficiency
MulticoreBenchmarkResult run_multicore_benchmark(
    const BatchConfig& config,
    std::vector<int> target_workers = {1, 2, 4}
);

// CSV export
std::string multicore_benchmark_to_csv(const MulticoreBenchmarkResult& res);

} // namespace workbench
