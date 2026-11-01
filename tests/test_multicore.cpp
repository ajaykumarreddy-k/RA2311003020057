#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "multicore_benchmark.hpp"
#include "thread_pool.hpp"
#include <atomic>

using namespace workbench;

TEST_CASE("Task 3 Multicore: ThreadPool Concurrency and Task Completion") {
    const size_t num_threads = 4;
    ThreadPool pool(num_threads);
    CHECK(pool.worker_count() == num_threads);

    std::atomic<int> counter{0};
    const int total_tasks = 100;
    std::vector<std::future<void>> futures;

    for (int i = 0; i < total_tasks; ++i) {
        futures.push_back(pool.enqueue([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    CHECK(counter.load() == total_tasks);
}

TEST_CASE("Task 3 Multicore: Seeded Batch Generator Determinism") {
    BatchConfig cfg1;
    cfg1.case_count = 50;
    cfg1.mesh_size = 20;
    cfg1.seed = 42;

    BatchConfig cfg2 = cfg1;

    auto batch1 = generate_seeded_batch(cfg1);
    auto batch2 = generate_seeded_batch(cfg2);

    REQUIRE(batch1.size() == 50);
    REQUIRE(batch2.size() == 50);

    for (size_t i = 0; i < batch1.size(); ++i) {
        CHECK(batch1[i].k == batch2[i].k);
        CHECK(batch1[i].q == batch2[i].q);
        CHECK(batch1[i].k > 0.0);
        CHECK(batch1[i].q > 0.0);
        CHECK(batch1[i].N == 20);
    }

    // Different seed gives different values
    BatchConfig cfg3 = cfg1;
    cfg3.seed = 999;
    auto batch3 = generate_seeded_batch(cfg3);
    CHECK(batch1[0].k != batch3[0].k);
}

TEST_CASE("Task 3 Multicore: Numerical Equivalence (Serial vs Parallel <= 1e-8)") {
    BatchConfig cfg;
    cfg.case_count = 100;
    cfg.mesh_size = 50;
    cfg.seed = 12345;

    auto batch = generate_seeded_batch(cfg);

    auto serial_res = solve_batch_serial(batch);
    auto parallel_res_2w = solve_batch_parallel(batch, 2);
    auto parallel_res_4w = solve_batch_parallel(batch, 4);

    REQUIRE(serial_res.size() == 100);
    REQUIRE(parallel_res_2w.size() == 100);
    REQUIRE(parallel_res_4w.size() == 100);

    double diff_2w = compare_solution_batches(serial_res, parallel_res_2w);
    double diff_4w = compare_solution_batches(serial_res, parallel_res_4w);

    CHECK(diff_2w <= 1e-8);
    CHECK(diff_4w <= 1e-8);
}

TEST_CASE("Task 3 Multicore: Benchmark Harness Execution and Metrics") {
    BatchConfig cfg;
    cfg.case_count = 150;
    cfg.mesh_size = 40;
    cfg.seed = 42;

    auto res = run_multicore_benchmark(cfg, {1, 2, 4});

    CHECK(res.verification_passed);
    CHECK(res.max_abs_diff_vs_serial <= 1e-8);
    CHECK(res.system.logical_cpus >= 1);
    CHECK(!res.system.compiler_name.empty());
    CHECK(!res.system.build_config.empty());

    REQUIRE(!res.runs.empty());

    // Worker 1 baseline
    CHECK(res.runs[0].worker_count == 1);
    CHECK(res.runs[0].repetition_times_ms.size() == 5);
    CHECK(res.runs[0].median_time_ms > 0.0);
    CHECK(res.runs[0].speedup == doctest::Approx(1.0));
    CHECK(res.runs[0].efficiency == doctest::Approx(1.0));

    // If multi-core machine, check 2 and 4 workers
    if (res.system.logical_cpus >= 2 && res.runs.size() >= 2) {
        CHECK(res.runs[1].worker_count == 2);
        CHECK(res.runs[1].median_time_ms > 0.0);
        CHECK(res.runs[1].speedup > 0.0);
        CHECK(res.runs[1].efficiency > 0.0);
    }

    if (res.system.logical_cpus >= 4 && res.runs.size() >= 3) {
        CHECK(res.runs[2].worker_count == 4);
        CHECK(res.runs[2].median_time_ms > 0.0);
        CHECK(res.runs[2].speedup > 0.0);
        CHECK(res.runs[2].efficiency > 0.0);
    }
}
