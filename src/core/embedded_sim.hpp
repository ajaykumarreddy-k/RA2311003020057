#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

namespace workbench {

enum class DeviceType {
    MICROCONTROLLER,
    FPGA,
    JTAG_ADAPTER
};

enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR_TIMEOUT,
    RECOVERING
};

inline std::string device_type_to_string(DeviceType type) {
    switch (type) {
        case DeviceType::MICROCONTROLLER: return "Microcontroller (MCU)";
        case DeviceType::FPGA:            return "FPGA Hardware Accelerator";
        case DeviceType::JTAG_ADAPTER:    return "JTAG Debug Adapter";
    }
    return "Unknown";
}

inline std::string connection_state_to_string(ConnectionState state) {
    switch (state) {
        case ConnectionState::DISCONNECTED:  return "DISCONNECTED";
        case ConnectionState::CONNECTING:    return "CONNECTING";
        case ConnectionState::CONNECTED:     return "CONNECTED";
        case ConnectionState::ERROR_TIMEOUT: return "ERROR_TIMEOUT";
        case ConnectionState::RECOVERING:    return "RECOVERING";
    }
    return "UNKNOWN";
}

struct DeviceTelemetry {
    std::string timestamp_iso;
    std::map<std::string, double> numeric_metrics;
    std::map<std::string, std::string> status_metrics;
};

struct DeviceInfo {
    std::string id;
    std::string name;
    DeviceType type;
    std::string transport;
    std::string serial_number;
    std::vector<std::string> capabilities;
    ConnectionState state;
    std::string last_error_message;
    int retry_count = 0;
};

class MockDeviceAdapter {
public:
    virtual ~MockDeviceAdapter() = default;

    virtual DeviceInfo get_info() const = 0;
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual DeviceTelemetry poll_telemetry() = 0;
    virtual bool trigger_failure() = 0;
    virtual bool retry_and_recover() = 0;
    virtual void update_simulation() = 0;
};

class McuMockAdapter : public MockDeviceAdapter {
public:
    McuMockAdapter();
    DeviceInfo get_info() const override;
    bool connect() override;
    bool disconnect() override;
    DeviceTelemetry poll_telemetry() override;
    bool trigger_failure() override;
    bool retry_and_recover() override;
    void update_simulation() override;

private:
    DeviceInfo info_;
    uint64_t sample_counter_ = 0;
    double core_temp_ = 42.5;
};

class FpgaMockAdapter : public MockDeviceAdapter {
public:
    FpgaMockAdapter();
    DeviceInfo get_info() const override;
    bool connect() override;
    bool disconnect() override;
    DeviceTelemetry poll_telemetry() override;
    bool trigger_failure() override;
    bool retry_and_recover() override;
    void update_simulation() override;

private:
    DeviceInfo info_;
    uint64_t cycle_counter_ = 0;
    double die_temp_ = 51.8;
};

class JtagMockAdapter : public MockDeviceAdapter {
public:
    JtagMockAdapter();
    DeviceInfo get_info() const override;
    bool connect() override;
    bool disconnect() override;
    DeviceTelemetry poll_telemetry() override;
    bool trigger_failure() override;
    bool retry_and_recover() override;
    void update_simulation() override;

private:
    DeviceInfo info_;
    double vref_sensed_ = 3.30;
};

class EmbeddedManager {
public:
    EmbeddedManager();

    std::vector<DeviceInfo> discover_all();
    DeviceInfo get_device_info(const std::string& id);
    bool connect_device(const std::string& id);
    bool disconnect_device(const std::string& id);
    DeviceTelemetry get_device_telemetry(const std::string& id);
    bool trigger_device_failure(const std::string& id);
    bool recover_device(const std::string& id);
    void update_all();

private:
    std::map<std::string, std::shared_ptr<MockDeviceAdapter>> devices_;
};

} // namespace workbench
