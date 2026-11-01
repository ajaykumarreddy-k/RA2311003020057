#pragma once

#include "multicore_benchmark.hpp"
#include "json.hpp"
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <map>

namespace workbench {

enum class JobStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    CANCELLED,
    FAILED
};

struct BenchmarkJob {
    std::string id;
    BatchConfig config;
    std::vector<int> target_workers;
    std::atomic<JobStatus> status{JobStatus::PENDING};
    std::atomic<int> progress_pct{0};
    std::string error_message;
    MulticoreBenchmarkResult result;
    std::string csv_data;
    std::atomic<bool> cancel_requested{false};
};

class JobManager {
public:
    JobManager() = default;
    ~JobManager() {
        cancel_all();
        for (auto& t : worker_threads_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    std::string start_benchmark_job(const BatchConfig& config, const std::vector<int>& target_workers) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string job_id = "job_" + std::to_string(job_counter_++);
        auto job = std::make_shared<BenchmarkJob>();
        job->id = job_id;
        job->config = config;
        job->target_workers = target_workers;
        job->status = JobStatus::RUNNING;
        job->progress_pct = 5;

        jobs_[job_id] = job;

        worker_threads_.emplace_back([job]() {
            try {
                job->progress_pct = 15;
                if (job->cancel_requested) {
                    job->status = JobStatus::CANCELLED;
                    return;
                }

                // Execute benchmark with cancellation checks
                job->progress_pct = 30;
                auto res = run_multicore_benchmark(job->config, job->target_workers);
                job->progress_pct = 85;

                if (job->cancel_requested) {
                    job->status = JobStatus::CANCELLED;
                    return;
                }

                job->result = res;
                job->csv_data = multicore_benchmark_to_csv(res);
                job->progress_pct = 100;
                job->status = JobStatus::COMPLETED;
            } catch (const std::exception& e) {
                job->status = JobStatus::FAILED;
                job->error_message = e.what();
            }
        });

        return job_id;
    }

    nlohmann::json get_job_status(const std::string& job_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = jobs_.find(job_id);
        if (it == jobs_.end()) {
            return {{"error", "Job not found: " + job_id}};
        }

        const auto& job = it->second;
        nlohmann::json j;
        j["job_id"] = job->id;
        j["progress_pct"] = job->progress_pct.load();

        switch (job->status.load()) {
            case JobStatus::PENDING:   j["status"] = "pending"; break;
            case JobStatus::RUNNING:   j["status"] = "running"; break;
            case JobStatus::COMPLETED: j["status"] = "completed"; break;
            case JobStatus::CANCELLED: j["status"] = "cancelled"; break;
            case JobStatus::FAILED:    j["status"] = "failed"; break;
        }

        if (job->status == JobStatus::FAILED) {
            j["error_message"] = job->error_message;
        } else if (job->status == JobStatus::COMPLETED) {
            j["csv_data"] = job->csv_data;
            j["system_info"] = {
                {"logical_cpus", job->result.system.logical_cpus},
                {"compiler_name", job->result.system.compiler_name},
                {"compiler_version", job->result.system.compiler_version},
                {"build_config", job->result.system.build_config},
                {"optimization_level", job->result.system.optimization_level},
                {"speedup_assessable", job->result.system.speedup_assessable}
            };
            j["verification"] = {
                {"max_abs_diff", job->result.max_abs_diff_vs_serial},
                {"passed", job->result.verification_passed}
            };

            nlohmann::json runs_json = nlohmann::json::array();
            for (const auto& r : job->result.runs) {
                runs_json.push_back({
                    {"workers", r.worker_count},
                    {"repetition_times_ms", r.repetition_times_ms},
                    {"median_time_ms", r.median_time_ms},
                    {"speedup", r.speedup},
                    {"efficiency", r.efficiency}
                });
            }
            j["runs"] = runs_json;
        }

        return j;
    }

    bool cancel_job(const std::string& job_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = jobs_.find(job_id);
        if (it != jobs_.end()) {
            it->second->cancel_requested = true;
            if (it->second->status == JobStatus::RUNNING) {
                it->second->status = JobStatus::CANCELLED;
            }
            return true;
        }
        return false;
    }

    void cancel_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [id, job] : jobs_) {
            job->cancel_requested = true;
        }
    }

private:
    std::mutex mutex_;
    int job_counter_ = 1;
    std::map<std::string, std::shared_ptr<BenchmarkJob>> jobs_;
    std::vector<std::thread> worker_threads_;
};

} // namespace workbench
