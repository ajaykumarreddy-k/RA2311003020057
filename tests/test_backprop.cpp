#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "backprop.hpp"
#include <cmath>

using namespace workbench;

TEST_CASE("Task 2 Backpropagation: Forward Pass at Initial Parameters") {
    double x = 1.0;
    double y = 2.0;
    BackpropWeights w0 = {0.5, 0.0, 1.0, 0.0};

    auto fp = forward_pass(x, y, w0);

    // z = 0.5 * 1.0 + 0.0 = 0.5
    CHECK(fp.z == doctest::Approx(0.5).epsilon(1e-12));

    // h = tanh(0.5) = 0.46211715726...
    double expected_h = std::tanh(0.5);
    CHECK(fp.h == doctest::Approx(expected_h).epsilon(1e-12));
    CHECK(fp.h == doctest::Approx(0.462117157).epsilon(1e-6));

    // y_hat = 1.0 * h + 0.0 = h
    CHECK(fp.y_hat == doctest::Approx(expected_h).epsilon(1e-12));

    // Error = y_hat - 2.0 = -1.537882843...
    double expected_err = expected_h - 2.0;
    CHECK((fp.y_hat - y) == doctest::Approx(expected_err).epsilon(1e-12));

    // Loss = 0.5 * (y_hat - 2)^2
    double expected_loss = 0.5 * expected_err * expected_err;
    CHECK(fp.loss == doctest::Approx(expected_loss).epsilon(1e-12));
    CHECK(fp.loss == doctest::Approx(1.18254182).epsilon(1e-6));
}

TEST_CASE("Task 2 Backpropagation: Analytic Gradients at Step 0") {
    double x = 1.0;
    double y = 2.0;
    BackpropWeights w0 = {0.5, 0.0, 1.0, 0.0};

    auto grads = compute_analytic_gradients(x, y, w0);

    double h = std::tanh(0.5);
    double e = h - 2.0; // -1.537882843
    double delta = e * 1.0 * (1.0 - h * h); // -1.209464593

    CHECK(grads.dc == doctest::Approx(e).epsilon(1e-12));
    CHECK(grads.dv == doctest::Approx(e * h).epsilon(1e-12));
    CHECK(grads.dw == doctest::Approx(delta * x).epsilon(1e-12));
    CHECK(grads.db == doctest::Approx(delta).epsilon(1e-12));

    CHECK(grads.dc == doctest::Approx(-1.53788284).epsilon(1e-6));
    CHECK(grads.dv == doctest::Approx(-0.71068205).epsilon(1e-6));
    CHECK(grads.dw == doctest::Approx(-1.20946459).epsilon(1e-6));
    CHECK(grads.db == doctest::Approx(-1.20946459).epsilon(1e-6));
}

TEST_CASE("Task 2 Backpropagation: Central Finite Difference Gradient Check (eps=1e-6, err<=1e-5)") {
    double x = 1.0;
    double y = 2.0;
    BackpropWeights w0 = {0.5, 0.0, 1.0, 0.0};

    auto report = check_gradients(x, y, w0, 1e-6, 1e-5);

    // Each gradient must match within 1e-5 tolerance
    CHECK(report.w_check.passed);
    CHECK(report.w_check.abs_error <= 1e-5);

    CHECK(report.b_check.passed);
    CHECK(report.b_check.abs_error <= 1e-5);

    CHECK(report.v_check.passed);
    CHECK(report.v_check.abs_error <= 1e-5);

    CHECK(report.c_check.passed);
    CHECK(report.c_check.abs_error <= 1e-5);

    CHECK(report.all_passed);
}

TEST_CASE("Task 2 Backpropagation: Simultaneous First Update and 100 Training Steps") {
    double x = 1.0;
    double y = 2.0;
    BackpropWeights w0 = {0.5, 0.0, 1.0, 0.0};
    double lr = 0.1;

    auto exp = run_backprop_experiment(x, y, w0, lr, 100);

    // Check weights after step 1
    // w1 = 0.5 - 0.1 * (-1.209464593) = 0.620946459
    CHECK(exp.weights_after_step_1.w == doctest::Approx(0.620946459).epsilon(1e-6));
    CHECK(exp.weights_after_step_1.b == doctest::Approx(0.120946459).epsilon(1e-6));
    CHECK(exp.weights_after_step_1.v == doctest::Approx(1.071068205).epsilon(1e-6));
    CHECK(exp.weights_after_step_1.c == doctest::Approx(0.153788284).epsilon(1e-6));

    // History contains 101 steps (0 to 100 inclusive)
    REQUIRE(exp.history.size() == 101);

    // Step 0 loss vs Step 100 loss
    double initial_loss = exp.history.front().loss;
    double final_loss = exp.history.back().loss;

    CHECK(initial_loss == doctest::Approx(1.18254182).epsilon(1e-6));
    CHECK(final_loss < 0.01);
    CHECK(final_loss < initial_loss);

    // Check monotonic decrease
    for (size_t s = 1; s < exp.history.size(); ++s) {
        CHECK(exp.history[s].loss <= exp.history[s - 1].loss + 1e-12);
    }
}
