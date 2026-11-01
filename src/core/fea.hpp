#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>

namespace workbench {

struct Fea1DParameters {
    double L = 1.0;          // Rod length in meters (m)
    double A = 1e-4;         // Cross-sectional area in square meters (m^2)
    double k = 10.0;         // Thermal conductivity in W/(m*K)
    double q = 10000.0;      // Heat generation rate in W/m^3
    double T_left = 100.0;   // Boundary condition at x=0 (deg C)
    double T_right = 20.0;   // Boundary condition at x=L (deg C)
    int N = 10;              // Number of linear finite elements

    void validate() const {
        if (L <= 0.0) {
            throw std::invalid_argument("Rod length L must be strictly positive (L > 0). Given: " + std::to_string(L));
        }
        if (A <= 0.0) {
            throw std::invalid_argument("Cross-sectional area A must be strictly positive (A > 0). Given: " + std::to_string(A));
        }
        if (k <= 0.0) {
            throw std::invalid_argument("Thermal conductivity k must be strictly positive (k > 0). Given: " + std::to_string(k));
        }
        if (N < 1) {
            throw std::invalid_argument("Number of elements N must be at least 1 (N >= 1). Given: " + std::to_string(N));
        }
    }
};

struct FeaNodeResult {
    double x = 0.0;
    double T_fea = 0.0;
    double T_exact = 0.0;
    double error = 0.0;
};

struct FeaMidpointResult {
    int element_idx = 0;
    double x_mid = 0.0;
    double T_fea_mid = 0.0;
    double T_exact_mid = 0.0;
    double error = 0.0;
};

struct Fea1DSolution {
    Fea1DParameters params;
    std::vector<FeaNodeResult> nodes;
    std::vector<FeaMidpointResult> midpoints;
    double max_node_error = 0.0;
    double max_midpoint_error = 0.0;
    double l2_midpoint_error = 0.0;
};

struct ConvergenceRow {
    int N = 0;
    double h = 0.0;
    double max_node_error = 0.0;
    double max_midpoint_error = 0.0;
    double observed_order = 0.0; // 0 for first entry
};

struct ConvergenceStudy {
    Fea1DParameters base_params;
    std::vector<int> mesh_sizes;
    std::vector<ConvergenceRow> rows;
    std::vector<Fea1DSolution> solutions;
};

// Analytical solution: T(x) = T_left - (T_left - T_right)*x/L + [q/(2k)]*x*(L - x)
inline double exact_temperature(double x, const Fea1DParameters& p) {
    double linear_part = p.T_left + (p.T_right - p.T_left) * (x / p.L);
    double source_part = (p.q / (2.0 * p.k)) * x * (p.L - x);
    return linear_part + source_part;
}

// Solve 1D steady-state heat conduction using 2-node linear elements
Fea1DSolution solve_fea_1d(const Fea1DParameters& params);

// Run convergence study across a vector of element counts (e.g. {10, 20, 40, 80})
ConvergenceStudy run_convergence_study(Fea1DParameters params, const std::vector<int>& mesh_sizes = {10, 20, 40, 80});

// CSV generation functions
std::string solution_to_csv(const Fea1DSolution& sol);
std::string convergence_to_csv(const ConvergenceStudy& study);

} // namespace workbench
