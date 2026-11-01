#include "backprop.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace workbench {

ForwardPassResult forward_pass(double x, double y, const BackpropWeights& weights) {
    ForwardPassResult res;
    res.x = x;
    res.y = y;
    res.z = weights.w * x + weights.b;
    res.h = std::tanh(res.z);
    res.y_hat = weights.v * res.h + weights.c;
    double diff = res.y_hat - y;
    res.loss = 0.5 * diff * diff;
    return res;
}

BackpropGradients compute_analytic_gradients(double x, double y, const BackpropWeights& weights) {
    ForwardPassResult fp = forward_pass(x, y, weights);

    // Error e = (y_hat - y)
    // Loss J = 0.5 * e^2
    // dJ/dy_hat = e
    double dJ_dy_hat = fp.y_hat - y;

    // Output layer gradients:
    // y_hat = v * h + c
    // dJ/dc = dJ/dy_hat * dy_hat/dc = dJ/dy_hat * 1
    double dJ_dc = dJ_dy_hat;

    // dJ/dv = dJ/dy_hat * dy_hat/dv = dJ/dy_hat * h
    double dJ_dv = dJ_dy_hat * fp.h;

    // Hidden layer gradients:
    // dJ/dh = dJ/dy_hat * dy_hat/dh = dJ/dy_hat * v
    double dJ_dh = dJ_dy_hat * weights.v;

    // h = tanh(z), dh/dz = 1 - tanh(z)^2 = 1 - h^2
    // delta = dJ/dz = dJ/dh * dh/dz = (y_hat - y) * v * (1 - h^2)
    double dh_dz = 1.0 - fp.h * fp.h;
    double delta = dJ_dh * dh_dz;

    // z = w * x + b
    // dJ/dw = delta * dz/dw = delta * x
    double dJ_dw = delta * x;

    // dJ/db = delta * dz/db = delta * 1
    double dJ_db = delta;

    BackpropGradients grads;
    grads.dw = dJ_dw;
    grads.db = dJ_db;
    grads.dv = dJ_dv;
    grads.dc = dJ_dc;
    return grads;
}

BackpropGradients compute_finite_difference_gradients(double x, double y, const BackpropWeights& weights, double eps) {
    BackpropGradients fd;

    // Check w
    {
        BackpropWeights w_plus = weights;
        BackpropWeights w_minus = weights;
        w_plus.w += eps;
        w_minus.w -= eps;
        double j_plus = forward_pass(x, y, w_plus).loss;
        double j_minus = forward_pass(x, y, w_minus).loss;
        fd.dw = (j_plus - j_minus) / (2.0 * eps);
    }

    // Check b
    {
        BackpropWeights b_plus = weights;
        BackpropWeights b_minus = weights;
        b_plus.b += eps;
        b_minus.b -= eps;
        double j_plus = forward_pass(x, y, b_plus).loss;
        double j_minus = forward_pass(x, y, b_minus).loss;
        fd.db = (j_plus - j_minus) / (2.0 * eps);
    }

    // Check v
    {
        BackpropWeights v_plus = weights;
        BackpropWeights v_minus = weights;
        v_plus.v += eps;
        v_minus.v -= eps;
        double j_plus = forward_pass(x, y, v_plus).loss;
        double j_minus = forward_pass(x, y, v_minus).loss;
        fd.dv = (j_plus - j_minus) / (2.0 * eps);
    }

    // Check c
    {
        BackpropWeights c_plus = weights;
        BackpropWeights c_minus = weights;
        c_plus.c += eps;
        c_minus.c -= eps;
        double j_plus = forward_pass(x, y, c_plus).loss;
        double j_minus = forward_pass(x, y, c_minus).loss;
        fd.dc = (j_plus - j_minus) / (2.0 * eps);
    }

    return fd;
}

BackpropCheckReport check_gradients(double x, double y, const BackpropWeights& weights, double eps, double tol) {
    BackpropGradients analytic = compute_analytic_gradients(x, y, weights);
    BackpropGradients fd = compute_finite_difference_gradients(x, y, weights, eps);

    BackpropCheckReport rep;

    rep.w_check.analytic = analytic.dw;
    rep.w_check.finite_diff = fd.dw;
    rep.w_check.abs_error = std::abs(analytic.dw - fd.dw);
    rep.w_check.passed = (rep.w_check.abs_error <= tol);

    rep.b_check.analytic = analytic.db;
    rep.b_check.finite_diff = fd.db;
    rep.b_check.abs_error = std::abs(analytic.db - fd.db);
    rep.b_check.passed = (rep.b_check.abs_error <= tol);

    rep.v_check.analytic = analytic.dv;
    rep.v_check.finite_diff = fd.dv;
    rep.v_check.abs_error = std::abs(analytic.dv - fd.dv);
    rep.v_check.passed = (rep.v_check.abs_error <= tol);

    rep.c_check.analytic = analytic.dc;
    rep.c_check.finite_diff = fd.dc;
    rep.c_check.abs_error = std::abs(analytic.dc - fd.dc);
    rep.c_check.passed = (rep.c_check.abs_error <= tol);

    rep.all_passed = rep.w_check.passed && rep.b_check.passed &&
                     rep.v_check.passed && rep.c_check.passed;

    return rep;
}

BackpropExperimentResult run_backprop_experiment(
    double x,
    double y,
    BackpropWeights initial_weights,
    double lr,
    int total_steps
) {
    BackpropExperimentResult result;
    result.initial_weights = initial_weights;
    result.initial_forward = forward_pass(x, y, initial_weights);
    result.initial_analytic_grads = compute_analytic_gradients(x, y, initial_weights);
    result.grad_check = check_gradients(x, y, initial_weights, 1e-6, 1e-5);

    BackpropWeights current_w = initial_weights;

    for (int step = 0; step <= total_steps; ++step) {
        ForwardPassResult fp = forward_pass(x, y, current_w);
        BackpropGradients grads = compute_analytic_gradients(x, y, current_w);

        TrainingStepRecord rec;
        rec.step = step;
        rec.w = current_w.w;
        rec.b = current_w.b;
        rec.v = current_w.v;
        rec.c = current_w.c;
        rec.y_hat = fp.y_hat;
        rec.loss = fp.loss;
        rec.dw = grads.dw;
        rec.db = grads.db;
        rec.dv = grads.dv;
        rec.dc = grads.dc;
        result.history.push_back(rec);

        if (step == 0) {
            // First simultaneous update using original-parameter gradients
            BackpropWeights updated_w;
            updated_w.w = current_w.w - lr * grads.dw;
            updated_w.b = current_w.b - lr * grads.db;
            updated_w.v = current_w.v - lr * grads.dv;
            updated_w.c = current_w.c - lr * grads.dc;
            result.weights_after_step_1 = updated_w;
            current_w = updated_w;
        } else if (step < total_steps) {
            // Further training steps
            current_w.w -= lr * grads.dw;
            current_w.b -= lr * grads.db;
            current_w.v -= lr * grads.dv;
            current_w.c -= lr * grads.dc;
        }
    }

    return result;
}

std::string backprop_history_to_csv(const BackpropExperimentResult& res) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << "# Backpropagation Training History (x=1.0, y=2.0, lr=0.1)\n";
    ss << "step,w,b,v,c,y_hat,loss,dw,db,dv,dc\n";

    for (const auto& r : res.history) {
        ss << r.step << "," << r.w << "," << r.b << "," << r.v << "," << r.c << ","
           << r.y_hat << "," << r.loss << "," << r.dw << "," << r.db << "," << r.dv << "," << r.dc << "\n";
    }

    return ss.str();
}

} // namespace workbench
