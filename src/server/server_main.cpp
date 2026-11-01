#include "httplib.h"
#include "json.hpp"
#include "fea.hpp"
#include "backprop.hpp"
#include "multicore_benchmark.hpp"
#include "stl_parser.hpp"
#include "ansys_parser.hpp"
#include "embedded_sim.hpp"
#include "job_manager.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using namespace workbench;

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string web_dir = "web";
    std::string data_dir = "data";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--web-dir" && i + 1 < argc) {
            web_dir = argv[++i];
        } else if (arg == "--data-dir" && i + 1 < argc) {
            data_dir = argv[++i];
        }
    }

    httplib::Server svr;

    // Static file serving from web directory
    svr.set_mount_point("/", web_dir);

    // Shared managers
    JobManager job_mgr;
    EmbeddedManager embedded_mgr;

    // Helper for CORS and JSON headers
    auto set_json_response = [](httplib::Response& res, const json& j, int status = 200) {
        res.status = status;
        res.set_content(j.dump(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    };

    // ---------------------------------------------------------
    // Health Check
    // ---------------------------------------------------------
    svr.Get("/api/health", [&](const httplib::Request&, httplib::Response& res) {
        json j = {
            {"status", "ok"},
            {"service", "Unified Engineering Workbench"},
            {"version", "1.0.0"}
        };
        set_json_response(res, j);
    });

    // ---------------------------------------------------------
    // Task 1: Thermal FEA Solve
    // ---------------------------------------------------------
    svr.Post("/api/fea/solve", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            Fea1DParameters p;
            p.L = body.value("L", 1.0);
            p.A = body.value("A", 1e-4);
            p.k = body.value("k", 10.0);
            p.q = body.value("q", 10000.0);
            p.T_left = body.value("T_left", 100.0);
            p.T_right = body.value("T_right", 20.0);
            p.N = body.value("N", 10);

            auto sol = solve_fea_1d(p);
            std::string csv = solution_to_csv(sol);

            json j;
            j["success"] = true;
            j["max_node_error"] = sol.max_node_error;
            j["max_midpoint_error"] = sol.max_midpoint_error;
            j["l2_midpoint_error"] = sol.l2_midpoint_error;
            j["csv_data"] = csv;

            json nodes_j = json::array();
            for (const auto& n : sol.nodes) {
                nodes_j.push_back({
                    {"x", n.x},
                    {"T_fea", n.T_fea},
                    {"T_exact", n.T_exact},
                    {"error", n.error}
                });
            }
            j["nodes"] = nodes_j;

            json mids_j = json::array();
            for (const auto& m : sol.midpoints) {
                mids_j.push_back({
                    {"elem", m.element_idx},
                    {"x_mid", m.x_mid},
                    {"T_fea_mid", m.T_fea_mid},
                    {"T_exact_mid", m.T_exact_mid},
                    {"error", m.error}
                });
            }
            j["midpoints"] = mids_j;

            set_json_response(res, j);
        } catch (const std::exception& e) {
            set_json_response(res, {{"success", false}, {"error", e.what()}}, 400);
        }
    });

    // ---------------------------------------------------------
    // Task 1: FEA Convergence Study
    // ---------------------------------------------------------
    svr.Post("/api/fea/convergence", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            Fea1DParameters p;
            p.L = body.value("L", 1.0);
            p.A = body.value("A", 1e-4);
            p.k = body.value("k", 10.0);
            p.q = body.value("q", 10000.0);
            p.T_left = body.value("T_left", 100.0);
            p.T_right = body.value("T_right", 20.0);

            std::vector<int> mesh_sizes = {10, 20, 40, 80};
            if (body.contains("mesh_sizes") && body["mesh_sizes"].is_array()) {
                mesh_sizes.clear();
                for (auto& item : body["mesh_sizes"]) {
                    mesh_sizes.push_back(item.get<int>());
                }
            }

            auto study = run_convergence_study(p, mesh_sizes);
            std::string csv = convergence_to_csv(study);

            json j;
            j["success"] = true;
            j["csv_data"] = csv;

            json rows_j = json::array();
            for (const auto& r : study.rows) {
                rows_j.push_back({
                    {"N", r.N},
                    {"h", r.h},
                    {"max_node_error", r.max_node_error},
                    {"max_midpoint_error", r.max_midpoint_error},
                    {"observed_order", r.observed_order}
                });
            }
            j["rows"] = rows_j;

            json curves_j = json::array();
            for (const auto& s : study.solutions) {
                json curve;
                curve["N"] = s.params.N;
                json pts = json::array();
                for (const auto& n : s.nodes) {
                    pts.push_back({{"x", n.x}, {"T_fea", n.T_fea}, {"T_exact", n.T_exact}});
                }
                curve["points"] = pts;
                curves_j.push_back(curve);
            }
            j["curves"] = curves_j;

            set_json_response(res, j);
        } catch (const std::exception& e) {
            set_json_response(res, {{"success", false}, {"error", e.what()}}, 400);
        }
    });

    // ---------------------------------------------------------
    // Task 2: Backpropagation
    // ---------------------------------------------------------
    svr.Post("/api/backprop/run", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            double x = body.value("x", 1.0);
            double y = body.value("y", 2.0);
            double lr = body.value("lr", 0.1);
            int steps = body.value("steps", 100);

            BackpropWeights w0;
            w0.w = body.value("w", 0.5);
            w0.b = body.value("b", 0.0);
            w0.v = body.value("v", 1.0);
            w0.c = body.value("c", 0.0);

            auto exp = run_backprop_experiment(x, y, w0, lr, steps);
            std::string csv = backprop_history_to_csv(exp);

            json j;
            j["success"] = true;
            j["csv_data"] = csv;

            j["initial_forward"] = {
                {"z", exp.initial_forward.z},
                {"h", exp.initial_forward.h},
                {"y_hat", exp.initial_forward.y_hat},
                {"loss", exp.initial_forward.loss}
            };

            j["initial_gradients"] = {
                {"dw", exp.initial_analytic_grads.dw},
                {"db", exp.initial_analytic_grads.db},
                {"dv", exp.initial_analytic_grads.dv},
                {"dc", exp.initial_analytic_grads.dc}
            };

            auto make_gc_json = [](const GradientCheckResult& r) {
                return json{
                    {"analytic", r.analytic},
                    {"finite_diff", r.finite_diff},
                    {"abs_error", r.abs_error},
                    {"passed", r.passed}
                };
            };

            j["gradient_check"] = {
                {"all_passed", exp.grad_check.all_passed},
                {"w", make_gc_json(exp.grad_check.w_check)},
                {"b", make_gc_json(exp.grad_check.b_check)},
                {"v", make_gc_json(exp.grad_check.v_check)},
                {"c", make_gc_json(exp.grad_check.c_check)}
            };

            j["weights_step_1"] = {
                {"w", exp.weights_after_step_1.w},
                {"b", exp.weights_after_step_1.b},
                {"v", exp.weights_after_step_1.v},
                {"c", exp.weights_after_step_1.c}
            };

            json hist_j = json::array();
            for (const auto& h : exp.history) {
                hist_j.push_back({
                    {"step", h.step},
                    {"w", h.w},
                    {"b", h.b},
                    {"v", h.v},
                    {"c", h.c},
                    {"y_hat", h.y_hat},
                    {"loss", h.loss},
                    {"dw", h.dw},
                    {"db", h.db},
                    {"dv", h.dv},
                    {"dc", h.dc}
                });
            }
            j["history"] = hist_j;

            set_json_response(res, j);
        } catch (const std::exception& e) {
            set_json_response(res, {{"success", false}, {"error", e.what()}}, 400);
        }
    });

    // ---------------------------------------------------------
    // Task 3: Multicore Asynchronous Benchmark
    // ---------------------------------------------------------
    svr.Post("/api/multicore/start", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            BatchConfig cfg;
            cfg.case_count = body.value("case_count", 500);
            cfg.mesh_size = body.value("mesh_size", 100);
            cfg.seed = body.value("seed", 42);

            std::vector<int> workers = {1, 2, 4};
            if (body.contains("workers") && body["workers"].is_array()) {
                workers.clear();
                for (auto& w : body["workers"]) {
                    workers.push_back(w.get<int>());
                }
            }

            std::string job_id = job_mgr.start_benchmark_job(cfg, workers);
            set_json_response(res, {{"success", true}, {"job_id", job_id}});
        } catch (const std::exception& e) {
            set_json_response(res, {{"success", false}, {"error", e.what()}}, 400);
        }
    });

    svr.Get("/api/multicore/poll", [&](const httplib::Request& req, httplib::Response& res) {
        std::string job_id = req.get_param_value("job_id");
        auto status_json = job_mgr.get_job_status(job_id);
        set_json_response(res, status_json);
    });

    svr.Post("/api/multicore/cancel", [&](const httplib::Request& req, httplib::Response& res) {
        std::string job_id = req.get_param_value("job_id");
        if (job_id.empty() && !req.body.empty()) {
            try {
                auto b = json::parse(req.body);
                job_id = b.value("job_id", "");
            } catch (...) {}
        }
        bool cancelled = job_mgr.cancel_job(job_id);
        set_json_response(res, {{"success", cancelled}, {"job_id", job_id}});
    });

    // ---------------------------------------------------------
    // Task 4: CAD & ANSYS Visualisation
    // ---------------------------------------------------------
    svr.Get("/api/geometry/load_synthetic_stl", [&](const httplib::Request&, httplib::Response& res) {
        try {
            std::string filepath = data_dir + "/synthetic_heat_sink.stl";
            auto model = parse_ascii_stl_file(filepath);

            json j;
            j["success"] = true;
            j["solid_name"] = model.solid_name;
            j["triangle_count"] = model.triangle_count();
            j["vertex_count"] = model.vertex_count();
            j["bbox_min"] = {model.bbox_min.x, model.bbox_min.y, model.bbox_min.z};
            j["bbox_max"] = {model.bbox_max.x, model.bbox_max.y, model.bbox_max.z};
            j["positions"] = model.flat_positions();
            j["normals"] = model.flat_normals();

            set_json_response(res, j);
        } catch (const std::exception& e) {
            set_json_response(res, {{"success", false}, {"error", e.what()}}, 400);
        }
    });

    svr.Get("/api/geometry/load_synthetic_ansys", [&](const httplib::Request&, httplib::Response& res) {
        try {
            std::string filepath = data_dir + "/synthetic_ansys_thermal.csv";
            auto mesh = parse_ansys_csv_file(filepath);

            json j;
            j["success"] = true;
            j["node_count"] = mesh.node_count();
            j["min_temperature"] = mesh.min_temperature;
            j["max_temperature"] = mesh.max_temperature;
            j["bbox_min"] = {mesh.bbox_min[0], mesh.bbox_min[1], mesh.bbox_min[2]};
            j["bbox_max"] = {mesh.bbox_max[0], mesh.bbox_max[1], mesh.bbox_max[2]};
            j["positions"] = mesh.flat_positions();
            j["temperatures"] = mesh.flat_temperatures();
            j["node_ids"] = mesh.flat_node_ids();

            set_json_response(res, j);
        } catch (const std::exception& e) {
            set_json_response(res, {{"success", false}, {"error", e.what()}}, 400);
        }
    });

    svr.Post("/api/geometry/parse_stl_text", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string stl_text = req.body;
            if (req.has_header("Content-Type") && req.get_header_value("Content-Type").find("application/json") != std::string::npos) {
                auto b = json::parse(req.body);
                stl_text = b.value("content", "");
            }
            auto model = parse_ascii_stl_string(stl_text);

            json j;
            j["success"] = true;
            j["solid_name"] = model.solid_name;
            j["triangle_count"] = model.triangle_count();
            j["vertex_count"] = model.vertex_count();
            j["bbox_min"] = {model.bbox_min.x, model.bbox_min.y, model.bbox_min.z};
            j["bbox_max"] = {model.bbox_max.x, model.bbox_max.y, model.bbox_max.z};
            j["positions"] = model.flat_positions();
            j["normals"] = model.flat_normals();

            set_json_response(res, j);
        } catch (const std::exception& e) {
            set_json_response(res, {{"success", false}, {"error", e.what()}}, 400);
        }
    });

    svr.Post("/api/geometry/parse_ansys_text", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string csv_text = req.body;
            if (req.has_header("Content-Type") && req.get_header_value("Content-Type").find("application/json") != std::string::npos) {
                auto b = json::parse(req.body);
                csv_text = b.value("content", "");
            }
            auto mesh = parse_ansys_csv_string(csv_text);

            json j;
            j["success"] = true;
            j["node_count"] = mesh.node_count();
            j["min_temperature"] = mesh.min_temperature;
            j["max_temperature"] = mesh.max_temperature;
            j["bbox_min"] = {mesh.bbox_min[0], mesh.bbox_min[1], mesh.bbox_min[2]};
            j["bbox_max"] = {mesh.bbox_max[0], mesh.bbox_max[1], mesh.bbox_max[2]};
            j["positions"] = mesh.flat_positions();
            j["temperatures"] = mesh.flat_temperatures();
            j["node_ids"] = mesh.flat_node_ids();

            set_json_response(res, j);
        } catch (const std::exception& e) {
            set_json_response(res, {{"success", false}, {"error", e.what()}}, 400);
        }
    });

    // ---------------------------------------------------------
    // Task 5: Embedded Integration (Simulated)
    // ---------------------------------------------------------
    svr.Get("/api/hardware/devices", [&](const httplib::Request&, httplib::Response& res) {
        auto devices = embedded_mgr.discover_all();
        json list = json::array();
        for (const auto& d : devices) {
            list.push_back({
                {"id", d.id},
                {"name", d.name},
                {"type", device_type_to_string(d.type)},
                {"transport", d.transport},
                {"serial_number", d.serial_number},
                {"capabilities", d.capabilities},
                {"state", connection_state_to_string(d.state)},
                {"last_error_message", d.last_error_message},
                {"retry_count", d.retry_count}
            });
        }
        set_json_response(res, {{"success", true}, {"devices", list}});
    });

    svr.Post("/api/hardware/connect", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = "";
        try {
            auto b = json::parse(req.body);
            id = b.value("id", "");
        } catch (...) {}
        bool ok = embedded_mgr.connect_device(id);
        set_json_response(res, {{"success", ok}, {"id", id}});
    });

    svr.Post("/api/hardware/disconnect", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = "";
        try {
            auto b = json::parse(req.body);
            id = b.value("id", "");
        } catch (...) {}
        bool ok = embedded_mgr.disconnect_device(id);
        set_json_response(res, {{"success", ok}, {"id", id}});
    });

    svr.Get("/api/hardware/telemetry", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.get_param_value("id");
        embedded_mgr.update_all();
        auto tel = embedded_mgr.get_device_telemetry(id);
        auto info = embedded_mgr.get_device_info(id);

        json j;
        j["success"] = true;
        j["id"] = id;
        j["state"] = connection_state_to_string(info.state);
        j["retry_count"] = info.retry_count;
        j["last_error_message"] = info.last_error_message;
        j["timestamp_iso"] = tel.timestamp_iso;
        j["numeric_metrics"] = tel.numeric_metrics;
        j["status_metrics"] = tel.status_metrics;

        set_json_response(res, j);
    });

    svr.Post("/api/hardware/trigger_failure", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = "";
        try {
            auto b = json::parse(req.body);
            id = b.value("id", "");
        } catch (...) {}
        bool ok = embedded_mgr.trigger_device_failure(id);
        set_json_response(res, {{"success", ok}, {"id", id}});
    });

    svr.Post("/api/hardware/recover", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = "";
        try {
            auto b = json::parse(req.body);
            id = b.value("id", "");
        } catch (...) {}
        bool ok = embedded_mgr.recover_device(id);
        set_json_response(res, {{"success", ok}, {"id", id}});
    });

    // ---------------------------------------------------------
    // Clean Shutdown API
    // ---------------------------------------------------------
    svr.Post("/api/shutdown", [&](const httplib::Request&, httplib::Response& res) {
        set_json_response(res, {{"status", "shutting_down"}});
        std::thread([&svr]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            svr.stop();
        }).detach();
    });

    std::cout << "[Workbench Server] Listening on http://127.0.0.1:" << port << std::endl;
    std::cout << "[Workbench Server] Serving web assets from '" << web_dir << "'..." << std::endl;

    svr.listen("127.0.0.1", port);

    std::cout << "[Workbench Server] Server stopped cleanly." << std::endl;
    return 0;
}
