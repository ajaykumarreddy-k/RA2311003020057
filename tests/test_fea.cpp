#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "fea.hpp"
#include <cmath>

using namespace workbench;

TEST_CASE("Task 1 FEA: Baseline Parameters and Boundary Conditions") {
    Fea1DParameters p;
    p.L = 1.0;
    p.A = 1e-4;
    p.k = 10.0;
    p.q = 10000.0;
    p.T_left = 100.0;
    p.T_right = 20.0;
    p.N = 10;

    auto sol = solve_fea_1d(p);

    // Verify boundary conditions
    CHECK(sol.nodes.front().x == doctest::Approx(0.0));
    CHECK(sol.nodes.front().T_fea == doctest::Approx(100.0).epsilon(1e-12));
    CHECK(sol.nodes.back().x == doctest::Approx(1.0));
    CHECK(sol.nodes.back().T_fea == doctest::Approx(20.0).epsilon(1e-12));
}

TEST_CASE("Task 1 FEA: Exact Node Values (Nodal Superconvergence)") {
    Fea1DParameters p;
    p.L = 1.0;
    p.A = 1e-4;
    p.k = 10.0;
    p.q = 10000.0;
    p.T_left = 100.0;
    p.T_right = 20.0;

    for (int N : {10, 20, 40, 80}) {
        p.N = N;
        auto sol = solve_fea_1d(p);

        // FEA with linear elements and piecewise constant source matches exact nodal values
        CHECK(sol.max_node_error < 1e-9);

        for (const auto& node : sol.nodes) {
            double t_exact = exact_temperature(node.x, p);
            CHECK(node.T_fea == doctest::Approx(t_exact).epsilon(1e-9));
        }
    }
}

TEST_CASE("Task 1 FEA: Midpoint Error Decreasing and Convergence Order 2.0") {
    Fea1DParameters p;
    p.L = 1.0;
    p.A = 1e-4;
    p.k = 10.0;
    p.q = 10000.0;
    p.T_left = 100.0;
    p.T_right = 20.0;

    auto study = run_convergence_study(p, {10, 20, 40, 80});

    REQUIRE(study.rows.size() == 4);

    // N=10 theoretical midpoint error: q/(8k) * h^2 = 125 * 0.01 = 1.25 C
    CHECK(study.rows[0].max_midpoint_error == doctest::Approx(1.25).epsilon(1e-6));

    // N=20 theoretical midpoint error: 125 * (0.05)^2 = 0.3125 C
    CHECK(study.rows[1].max_midpoint_error == doctest::Approx(0.3125).epsilon(1e-6));

    // N=40 theoretical midpoint error: 125 * (0.025)^2 = 0.078125 C
    CHECK(study.rows[2].max_midpoint_error == doctest::Approx(0.078125).epsilon(1e-6));

    // N=80 theoretical midpoint error: 125 * (0.0125)^2 = 0.01953125 C
    CHECK(study.rows[3].max_midpoint_error == doctest::Approx(0.01953125).epsilon(1e-6));

    // Midpoint error strictly decreasing with N
    CHECK(study.rows[1].max_midpoint_error < study.rows[0].max_midpoint_error);
    CHECK(study.rows[2].max_midpoint_error < study.rows[1].max_midpoint_error);
    CHECK(study.rows[3].max_midpoint_error < study.rows[2].max_midpoint_error);

    // Observed order must be 2.0 (O(h^2) convergence)
    for (size_t i = 1; i < study.rows.size(); ++i) {
        CHECK(study.rows[i].observed_order == doctest::Approx(2.0).epsilon(1e-4));
    }
}

TEST_CASE("Task 1 FEA: Invalid Parameter Rejections") {
    Fea1DParameters p;

    // k <= 0
    p.k = 0.0;
    CHECK_THROWS_AS(p.validate(), std::invalid_argument);
    CHECK_THROWS_AS(solve_fea_1d(p), std::invalid_argument);
    p.k = -5.0;
    CHECK_THROWS_AS(solve_fea_1d(p), std::invalid_argument);
    p.k = 10.0;

    // L <= 0
    p.L = 0.0;
    CHECK_THROWS_AS(solve_fea_1d(p), std::invalid_argument);
    p.L = -1.0;
    CHECK_THROWS_AS(solve_fea_1d(p), std::invalid_argument);
    p.L = 1.0;

    // A <= 0
    p.A = 0.0;
    CHECK_THROWS_AS(solve_fea_1d(p), std::invalid_argument);
    p.A = -1e-4;
    CHECK_THROWS_AS(solve_fea_1d(p), std::invalid_argument);
    p.A = 1e-4;

    // N < 1
    p.N = 0;
    CHECK_THROWS_AS(solve_fea_1d(p), std::invalid_argument);
    p.N = -5;
    CHECK_THROWS_AS(solve_fea_1d(p), std::invalid_argument);
}

TEST_CASE("Task 1 FEA: Single Element N=1 Edge Case") {
    Fea1DParameters p;
    p.N = 1;
    auto sol = solve_fea_1d(p);
    CHECK(sol.nodes.size() == 2);
    CHECK(sol.nodes[0].T_fea == doctest::Approx(100.0));
    CHECK(sol.nodes[1].T_fea == doctest::Approx(20.0));
}
