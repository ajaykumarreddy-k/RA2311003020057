#pragma once

#include <vector>
#include <string>

namespace workbench {

struct BackpropWeights {
    double w = 0.5;
    double b = 0.0;
    double v = 1.0;
    double c = 0.0;
};

struct BackpropGradients {
    double dw = 0.0;
    double db = 0.0;
    double dv = 0.0;
    double dc = 0.0;
};

struct ForwardPassResult {
    double x = 1.0;
    double y = 2.0;
    double z = 0.0;       // z = w*x + b
    double h = 0.0;       // h = tanh(z)
    double y_hat = 0.0;   // y_hat = v*h + c
    double loss = 0.0;    // J = 0.5 * (y_hat - y)^2
};

struct GradientCheckResult {
    double analytic = 0.0;
    double finite_diff = 0.0;
    double abs_error = 0.0;
    bool passed = false;
};

struct BackpropCheckReport {
    GradientCheckResult w_check;
    GradientCheckResult b_check;
    GradientCheckResult v_check;
    GradientCheckResult c_check;
    bool all_passed = false;
};

struct TrainingStepRecord {
    int step = 0;
    double w = 0.0;
    double b = 0.0;
    double v = 0.0;
    double c = 0.0;
    double y_hat = 0.0;
    double loss = 0.0;
    double dw = 0.0;
    double db = 0.0;
    double dv = 0.0;
    double dc = 0.0;
};

struct BackpropExperimentResult {
    BackpropWeights initial_weights;
    ForwardPassResult initial_forward;
    BackpropGradients initial_analytic_grads;
    BackpropCheckReport grad_check;
    BackpropWeights weights_after_step_1;
    std::vector<TrainingStepRecord> history; // 0 to num_steps
};

// Forward pass computation
ForwardPassResult forward_pass(double x, double y, const BackpropWeights& weights);

// Analytic gradients computation
BackpropGradients compute_analytic_gradients(double x, double y, const BackpropWeights& weights);

// Finite difference gradient computation
BackpropGradients compute_finite_difference_gradients(double x, double y, const BackpropWeights& weights, double eps = 1e-6);

// Gradient checking (analytic vs finite difference with tolerance <= 1e-5)
BackpropCheckReport check_gradients(double x, double y, const BackpropWeights& weights, double eps = 1e-6, double tol = 1e-5);

// Run full experiment: forward, grad check, simultaneous update, then 100 further steps (total 101 steps)
BackpropExperimentResult run_backprop_experiment(
    double x = 1.0,
    double y = 2.0,
    BackpropWeights initial_weights = {0.5, 0.0, 1.0, 0.0},
    double lr = 0.1,
    int total_steps = 100
);

// CSV export
std::string backprop_history_to_csv(const BackpropExperimentResult& res);

} // namespace workbench
