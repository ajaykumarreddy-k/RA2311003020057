#include "fea.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace workbench {

Fea1DSolution solve_fea_1d(const Fea1DParameters& params) {
    params.validate();

    const int N = params.N;
    const int num_nodes = N + 1;
    const double h = params.L / static_cast<double>(N);

    // Tridiagonal system representation:
    // D: main diagonal [0 .. num_nodes-1]
    // L_diag: sub-diagonal [0 .. num_nodes-2], L_diag[i] connects node i+1 to node i
    // U_diag: super-diagonal [0 .. num_nodes-2], U_diag[i] connects node i to node i+1
    // F: right hand side vector [0 .. num_nodes-1]
    std::vector<double> D(num_nodes, 0.0);
    std::vector<double> L_diag(num_nodes - 1, 0.0);
    std::vector<double> U_diag(num_nodes - 1, 0.0);
    std::vector<double> F(num_nodes, 0.0);

    // Element-level constants for uniform mesh:
    // k_elem = (k * A) / h * [ 1 -1; -1 1 ]
    // f_elem = (q * A * h) / 2 * [ 1; 1 ]
    const double k_elem = (params.k * params.A) / h;
    const double f_elem = (params.q * params.A * h) * 0.5;

    // Assembly loop over elements
    for (int e = 0; e < N; ++e) {
        int i = e;
        int j = e + 1;

        D[i] += k_elem;
        D[j] += k_elem;

        U_diag[i] += -k_elem;
        L_diag[i] += -k_elem;

        F[i] += f_elem;
        F[j] += f_elem;
    }

    // Apply Dirichlet Boundary Conditions:
    // Node 0: T(0) = params.T_left
    // Node N: T(L) = params.T_right
    //
    // Using symmetric boundary condition enforcement:
    // For node 0:
    // Subtract column 0 contribution from adjacent equation: F[1] -= L_diag[0] * T_left
    // Set row 0: D[0] = 1, U_diag[0] = 0, F[0] = T_left
    // Set L_diag[0] = 0 (column elimination)
    F[1] -= L_diag[0] * params.T_left;
    D[0] = 1.0;
    U_diag[0] = 0.0;
    L_diag[0] = 0.0;
    F[0] = params.T_left;

    // For node N:
    // Subtract column N contribution from adjacent equation: F[N-1] -= U_diag[N-2] * T_right
    // Set row N: D[N] = 1, L_diag[N-2] = 0, F[N] = T_right
    // Set U_diag[N-2] = 0
    if (N > 1) {
        F[N - 1] -= U_diag[N - 1] * params.T_right;
        D[N] = 1.0;
        L_diag[N - 1] = 0.0;
        U_diag[N - 1] = 0.0;
        F[N] = params.T_right;
    } else {
        // Edge case: N=1 (single element with 2 nodes, both Dirichlet BCs)
        D[1] = 1.0;
        F[1] = params.T_right;
    }

    // Thomas Algorithm (tridiagonal matrix algorithm)
    // Forward elimination
    std::vector<double> c_prime(num_nodes - 1, 0.0);
    std::vector<double> d_prime(num_nodes, 0.0);

    c_prime[0] = U_diag[0] / D[0];
    d_prime[0] = F[0] / D[0];

    for (int i = 1; i < num_nodes - 1; ++i) {
        double denom = D[i] - L_diag[i - 1] * c_prime[i - 1];
        c_prime[i] = U_diag[i] / denom;
        d_prime[i] = (F[i] - L_diag[i - 1] * d_prime[i - 1]) / denom;
    }

    if (num_nodes > 1) {
        int last = num_nodes - 1;
        double denom = D[last] - L_diag[last - 1] * c_prime[last - 1];
        d_prime[last] = (F[last] - L_diag[last - 1] * d_prime[last - 1]) / denom;
    }

    // Back substitution
    std::vector<double> T(num_nodes, 0.0);
    T[num_nodes - 1] = d_prime[num_nodes - 1];
    for (int i = num_nodes - 2; i >= 0; --i) {
        T[i] = d_prime[i] - c_prime[i] * T[i + 1];
    }

    // Populate solution structure
    Fea1DSolution sol;
    sol.params = params;
    sol.nodes.resize(num_nodes);
    sol.max_node_error = 0.0;

    for (int i = 0; i < num_nodes; ++i) {
        double x_i = static_cast<double>(i) * h;
        double t_exact_i = exact_temperature(x_i, params);
        double err = std::abs(T[i] - t_exact_i);

        sol.nodes[i] = { x_i, T[i], t_exact_i, err };
        if (err > sol.max_node_error) {
            sol.max_node_error = err;
        }
    }

    // Element midpoints evaluation using linear FE interpolation:
    // T_mid(e) = 0.5 * (T[e] + T[e+1])
    sol.midpoints.resize(N);
    sol.max_midpoint_error = 0.0;
    double sum_sq_err = 0.0;

    for (int e = 0; e < N; ++e) {
        double x_mid = (static_cast<double>(e) + 0.5) * h;
        double t_fea_mid = 0.5 * (T[e] + T[e + 1]);
        double t_exact_mid = exact_temperature(x_mid, params);
        double err_mid = std::abs(t_fea_mid - t_exact_mid);

        sol.midpoints[e] = { e, x_mid, t_fea_mid, t_exact_mid, err_mid };
        if (err_mid > sol.max_midpoint_error) {
            sol.max_midpoint_error = err_mid;
        }
        sum_sq_err += err_mid * err_mid;
    }
    sol.l2_midpoint_error = std::sqrt(sum_sq_err / static_cast<double>(N));

    return sol;
}

ConvergenceStudy run_convergence_study(Fea1DParameters params, const std::vector<int>& mesh_sizes) {
    ConvergenceStudy study;
    study.base_params = params;
    study.mesh_sizes = mesh_sizes;

    for (size_t idx = 0; idx < mesh_sizes.size(); ++idx) {
        int n_elems = mesh_sizes[idx];
        params.N = n_elems;
        Fea1DSolution sol = solve_fea_1d(params);
        study.solutions.push_back(sol);

        ConvergenceRow row;
        row.N = n_elems;
        row.h = params.L / static_cast<double>(n_elems);
        row.max_node_error = sol.max_node_error;
        row.max_midpoint_error = sol.max_midpoint_error;

        if (idx > 0) {
            double prev_err = study.rows[idx - 1].max_midpoint_error;
            double curr_err = row.max_midpoint_error;
            double prev_N = static_cast<double>(study.rows[idx - 1].N);
            double curr_N = static_cast<double>(n_elems);

            if (curr_err > 1e-15 && prev_err > 1e-15) {
                row.observed_order = std::log(prev_err / curr_err) / std::log(curr_N / prev_N);
            } else {
                row.observed_order = 0.0;
            }
        } else {
            row.observed_order = 0.0;
        }

        study.rows.push_back(row);
    }

    return study;
}

std::string solution_to_csv(const Fea1DSolution& sol) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << "# 1D Thermal FEA Solution (N=" << sol.params.N
       << ", L=" << sol.params.L << " m, k=" << sol.params.k
       << " W/(m*K), q=" << sol.params.q << " W/m^3)\n";
    ss << "type,index,x_m,T_fea_C,T_exact_C,error_C\n";

    for (size_t i = 0; i < sol.nodes.size(); ++i) {
        const auto& n = sol.nodes[i];
        ss << "node," << i << "," << n.x << "," << n.T_fea << "," << n.T_exact << "," << n.error << "\n";
    }

    for (size_t e = 0; e < sol.midpoints.size(); ++e) {
        const auto& m = sol.midpoints[e];
        ss << "midpoint," << m.element_idx << "," << m.x_mid << "," << m.T_fea_mid << "," << m.T_exact_mid << "," << m.error << "\n";
    }

    return ss.str();
}

std::string convergence_to_csv(const ConvergenceStudy& study) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << "# FEA Convergence Study (L=" << study.base_params.L
       << " m, k=" << study.base_params.k << " W/(m*K), q="
       << study.base_params.q << " W/m^3)\n";
    ss << "N,h_m,max_node_error_C,max_midpoint_error_C,observed_order\n";

    for (const auto& r : study.rows) {
        ss << r.N << "," << r.h << "," << r.max_node_error << "," << r.max_midpoint_error << ",";
        if (r.observed_order > 0.0) {
            ss << r.observed_order;
        } else {
            ss << "N/A";
        }
        ss << "\n";
    }

    return ss.str();
}

} // namespace workbench
