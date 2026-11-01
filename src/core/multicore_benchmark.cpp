#include "multicore_benchmark.hpp"
#include "thread_pool.hpp"
#include <random>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace workbench {

SystemInfo get_system_info() {
    SystemInfo info;
    info.logical_cpus = std::thread::hardware_concurrency();
    if (info.logical_cpus == 0) {
        info.logical_cpus = 1;
    }
    info.speedup_assessable = (info.logical_cpus > 1);

#if defined(__clang__)
    info.compiler_name = "Clang";
    info.compiler_version = __clang_version__;
#elif defined(__GNUC__) || defined(__GNUG__)
    info.compiler_name = "GCC";
    info.compiler_version = __VERSION__;
#elif defined(_MSC_VER)
    info.compiler_name = "MSVC";
    info.compiler_version = std::to_string(_MSC_VER);
#else
    info.compiler_name = "Unknown";
    info.compiler_version = "Unknown";
#endif

#if defined(NDEBUG)
    info.build_config = "Release";
#else
    info.build_config = "Debug";
#endif

#if defined(__OPTIMIZE__)
    info.optimization_level = "Optimized (-O2 / -O3)";
#elif defined(_MSC_VER) && !defined(_DEBUG)
    info.optimization_level = "Optimized (/O2 / /Ox)";
#else
    info.optimization_level = "No optimization (-O0 / /Od)";
#endif

    return info;
}

std::vector<Fea1DParameters> generate_seeded_batch(const BatchConfig& config) {
    std::mt19937 rng(config.seed);
    std::uniform_real_distribution<double> dist_k(config.k_min, config.k_max);
    std::uniform_real_distribution<double> dist_q(config.q_min, config.q_max);

    std::vector<Fea1DParameters> batch;
    batch.reserve(config.case_count);

    for (int i = 0; i < config.case_count; ++i) {
        Fea1DParameters p;
        p.L = 1.0;
        p.A = 1e-4;
        p.k = dist_k(rng);
        p.q = dist_q(rng);
        p.T_left = 100.0;
        p.T_right = 20.0;
        p.N = config.mesh_size;
        batch.push_back(p);
    }

    return batch;
}

std::vector<Fea1DSolution> solve_batch_serial(const std::vector<Fea1DParameters>& batch) {
    std::vector<Fea1DSolution> results;
    results.reserve(batch.size());

    for (const auto& p : batch) {
        results.push_back(solve_fea_1d(p));
    }

    return results;
}

std::vector<Fea1DSolution> solve_batch_parallel(const std::vector<Fea1DParameters>& batch, int num_workers) {
    if (num_workers <= 1) {
        return solve_batch_serial(batch);
    }

    const size_t total_cases = batch.size();
    std::vector<Fea1DSolution> results(total_cases);

    ThreadPool pool(static_cast<size_t>(num_workers));
    std::vector<std::future<void>> futures;

    // Divide batch into chunks across workers for low scheduling overhead
    size_t chunk_size = (total_cases + num_workers - 1) / num_workers;
    if (chunk_size == 0) chunk_size = 1;

    for (size_t start = 0; start < total_cases; start += chunk_size) {
        size_t end = std::min(start + chunk_size, total_cases);
        futures.push_back(pool.enqueue([&batch, &results, start, end]() {
            for (size_t i = start; i < end; ++i) {
                results[i] = solve_fea_1d(batch[i]);
            }
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    return results;
}

double compare_solution_batches(
    const std::vector<Fea1DSolution>& sol_a,
    const std::vector<Fea1DSolution>& sol_b
) {
    if (sol_a.size() != sol_b.size()) {
        return 1e9; // Large sentinel
    }

    double max_diff = 0.0;
    for (size_t c = 0; c < sol_a.size(); ++c) {
        const auto& nodes_a = sol_a[c].nodes;
        const auto& nodes_b = sol_b[c].nodes;
        if (nodes_a.size() != nodes_b.size()) {
            return 1e9;
        }
        for (size_t n = 0; n < nodes_a.size(); ++n) {
            double d = std::abs(nodes_a[n].T_fea - nodes_b[n].T_fea);
            if (d > max_diff) {
                max_diff = d;
            }
        }
    }
    return max_diff;
}

MulticoreBenchmarkResult run_multicore_benchmark(
    const BatchConfig& config,
    std::vector<int> target_workers
) {
    MulticoreBenchmarkResult result;
    result.config = config;
    result.system = get_system_info();

    // Generate batch
    auto batch = generate_seeded_batch(config);

    // Filter and cap target worker counts
    std::vector<int> valid_workers;
    for (int w : target_workers) {
        if (w >= 1 && w <= static_cast<int>(result.system.logical_cpus)) {
            if (std::find(valid_workers.begin(), valid_workers.end(), w) == valid_workers.end()) {
                valid_workers.push_back(w);
            }
        }
    }
    if (valid_workers.empty()) {
        valid_workers.push_back(1);
    }
    std::sort(valid_workers.begin(), valid_workers.end());

    // Compute baseline serial solutions for verification
    auto serial_solutions = solve_batch_serial(batch);

    // Warm-up parallel execution (run with max requested worker count)
    int max_workers = valid_workers.back();
    auto warmup_solutions = solve_batch_parallel(batch, max_workers);

    // Verify serial vs parallel identicality
    result.max_abs_diff_vs_serial = compare_solution_batches(serial_solutions, warmup_solutions);
    result.verification_passed = (result.max_abs_diff_vs_serial <= 1e-8);

    double t1_median = 0.0;

    for (int w : valid_workers) {
        MulticoreWorkerResult wr;
        wr.worker_count = w;

        // 1 warm-up run per worker count
        (void)solve_batch_parallel(batch, w);

        // 5 measured repetitions (timing only compute)
        std::vector<double> rep_times;
        for (int rep = 0; rep < 5; ++rep) {
            auto t_start = std::chrono::steady_clock::now();
            auto res = solve_batch_parallel(batch, w);
            auto t_end = std::chrono::steady_clock::now();

            double ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
            rep_times.push_back(ms);
        }

        wr.repetition_times_ms = rep_times;

        // Calculate median time
        std::vector<double> sorted_times = rep_times;
        std::sort(sorted_times.begin(), sorted_times.end());
        wr.median_time_ms = sorted_times[sorted_times.size() / 2];

        if (w == 1) {
            t1_median = wr.median_time_ms;
            wr.speedup = 1.0;
            wr.efficiency = 1.0;
        } else {
            if (t1_median > 0.0 && wr.median_time_ms > 0.0) {
                wr.speedup = t1_median / wr.median_time_ms;
                wr.efficiency = wr.speedup / static_cast<double>(w);
            } else {
                wr.speedup = 1.0;
                wr.efficiency = 1.0;
            }
        }

        result.runs.push_back(wr);
    }

    return result;
}

std::string multicore_benchmark_to_csv(const MulticoreBenchmarkResult& res) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "# Multicore Benchmark Results\n";
    ss << "# Logical CPUs: " << res.system.logical_cpus << "\n";
    ss << "# Compiler: " << res.system.compiler_name << " " << res.system.compiler_version << "\n";
    ss << "# Build Config: " << res.system.build_config << " (" << res.system.optimization_level << ")\n";
    ss << "# Cases: " << res.config.case_count << ", Mesh size N: " << res.config.mesh_size << ", Seed: " << res.config.seed << "\n";
    ss << "# Verification Max Abs Difference: " << std::scientific << res.max_abs_diff_vs_serial
       << " (Passed: " << (res.verification_passed ? "YES" : "NO") << ")\n";
    ss << std::fixed;
    ss << "workers,rep1_ms,rep2_ms,rep3_ms,rep4_ms,rep5_ms,median_ms,speedup,efficiency\n";

    for (const auto& r : res.runs) {
        ss << r.worker_count;
        for (double t : r.repetition_times_ms) {
            ss << "," << t;
        }
        ss << "," << r.median_time_ms << "," << r.speedup << "," << r.efficiency << "\n";
    }

    return ss.str();
}

} // namespace workbench
