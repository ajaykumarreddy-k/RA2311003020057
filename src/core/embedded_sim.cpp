#include "embedded_sim.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>

namespace workbench {

static std::string get_current_time_iso() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm gm{};
#if defined(_WIN32)
    gmtime_s(&gm, &tt);
#else
    gmtime_r(&tt, &gm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&gm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

// -------------------------------------------------------------
// McuMockAdapter (STM32H743ZI Cortex-M7)
// -------------------------------------------------------------
McuMockAdapter::McuMockAdapter() {
    info_.id = "mcu_stm32h7";
    info_.name = "STM32H743ZI Cortex-M7 Microcontroller";
    info_.type = DeviceType::MICROCONTROLLER;
    info_.transport = "USB CDC-ACM / Virtual COM Port (115200 baud)";
    info_.serial_number = "STM32-H7-43ZI-7A890F";
    info_.capabilities = {
        "ADC_SAMPLING (16-bit, 3.6 MSPS)",
        "PWM_MOTOR_CONTROL",
        "HARDWARE_THERMAL_WATCHDOG",
        "EMBEDDED_FLASH_STORAGE (2 MB)",
        "FREE_RTOS_PREEMPTIVE_SCHEDULER"
    };
    info_.state = ConnectionState::DISCONNECTED;
    info_.last_error_message = "None";
    info_.retry_count = 0;
}

DeviceInfo McuMockAdapter::get_info() const {
    return info_;
}

bool McuMockAdapter::connect() {
    info_.state = ConnectionState::CONNECTED;
    info_.last_error_message = "None";
    return true;
}

bool McuMockAdapter::disconnect() {
    info_.state = ConnectionState::DISCONNECTED;
    return true;
}

DeviceTelemetry McuMockAdapter::poll_telemetry() {
    DeviceTelemetry tel;
    tel.timestamp_iso = get_current_time_iso();

    if (info_.state != ConnectionState::CONNECTED) {
        tel.status_metrics["status"] = connection_state_to_string(info_.state);
        tel.status_metrics["error"] = info_.last_error_message;
        return tel;
    }

    sample_counter_ += 10;
    // Add small random noise to temperature
    double noise = (((sample_counter_ % 17) - 8.0) * 0.05);

    tel.numeric_metrics["core_temperature_C"] = core_temp_ + noise;
    tel.numeric_metrics["vcc_core_V"] = 1.201;
    tel.numeric_metrics["vcc_io_3v3_V"] = 3.298;
    tel.numeric_metrics["clock_freq_MHz"] = 480.0;
    tel.numeric_metrics["samples_processed"] = static_cast<double>(sample_counter_);

    tel.status_metrics["firmware_version"] = "v2.4.1-rc3";
    tel.status_metrics["operating_mode"] = "RUN_HIGH_PERFORMANCE";
    tel.status_metrics["watchdog_status"] = "ACTIVE_OK";

    return tel;
}

bool McuMockAdapter::trigger_failure() {
    info_.state = ConnectionState::ERROR_TIMEOUT;
    info_.last_error_message = "UART Rx Timeout: ACK packet frame 0xA5 not received within 2000 ms. Buffer overrun suspected.";
    return true;
}

bool McuMockAdapter::retry_and_recover() {
    info_.retry_count++;
    info_.state = ConnectionState::RECOVERING;
    // Perform flush and reconnect handshake
    info_.state = ConnectionState::CONNECTED;
    info_.last_error_message = "Recovered: UART circular buffer flushed, handshake resynchronized.";
    return true;
}

void McuMockAdapter::update_simulation() {
    if (info_.state == ConnectionState::CONNECTED) {
        sample_counter_ += 5;
    }
}

// -------------------------------------------------------------
// FpgaMockAdapter (Xilinx Artix-7 XC7A100T)
// -------------------------------------------------------------
FpgaMockAdapter::FpgaMockAdapter() {
    info_.id = "fpga_artix7";
    info_.name = "Xilinx Artix-7 XC7A100T Accelerator";
    info_.type = DeviceType::FPGA;
    info_.transport = "Memory-Mapped SPI / PCIe Subsystem (10 MHz)";
    info_.serial_number = "XLX-A7-100T-9921BC";
    info_.capabilities = {
        "FEA_ACCELERATOR_CORE (Matrix Assembly Pipeline)",
        "PARALLEL_FFT_ENGINE_1024",
        "AXI4_STREAM_DMA_FIFO",
        "RECONFIGURABLE_DSP48E1_SLICES (240 units)",
        "INTERNAL_SYSMON_ADC"
    };
    info_.state = ConnectionState::DISCONNECTED;
    info_.last_error_message = "None";
    info_.retry_count = 0;
}

DeviceInfo FpgaMockAdapter::get_info() const {
    return info_;
}

bool FpgaMockAdapter::connect() {
    info_.state = ConnectionState::CONNECTED;
    info_.last_error_message = "None";
    return true;
}

bool FpgaMockAdapter::disconnect() {
    info_.state = ConnectionState::DISCONNECTED;
    return true;
}

DeviceTelemetry FpgaMockAdapter::poll_telemetry() {
    DeviceTelemetry tel;
    tel.timestamp_iso = get_current_time_iso();

    if (info_.state != ConnectionState::CONNECTED) {
        tel.status_metrics["status"] = connection_state_to_string(info_.state);
        tel.status_metrics["error"] = info_.last_error_message;
        return tel;
    }

    cycle_counter_ += 10000;
    double noise = (((cycle_counter_ % 23) - 11.0) * 0.08);

    tel.numeric_metrics["die_temperature_C"] = die_temp_ + noise;
    tel.numeric_metrics["vcc_int_1v0_V"] = 1.002;
    tel.numeric_metrics["vcc_aux_1v8_V"] = 1.799;
    tel.numeric_metrics["vcc_bram_1v0_V"] = 0.998;
    tel.numeric_metrics["lut_utilization_pct"] = 41.2;
    tel.numeric_metrics["dsp_utilization_pct"] = 68.5;
    tel.numeric_metrics["clock_freq_MHz"] = 200.0;
    tel.numeric_metrics["compute_cycles"] = static_cast<double>(cycle_counter_);

    tel.status_metrics["bitstream_id"] = "FEA_ACCEL_V1_OPT";
    tel.status_metrics["sysmon_alarm"] = "ALL_LIMITS_NOMINAL";
    tel.status_metrics["dna_identifier"] = "0x400200000128919A";

    return tel;
}

bool FpgaMockAdapter::trigger_failure() {
    info_.state = ConnectionState::ERROR_TIMEOUT;
    info_.last_error_message = "AXI4-Stream CRC Mismatch: DMA transfer check failed on block 0x3F. Incomplete packet burst.";
    return true;
}

bool FpgaMockAdapter::retry_and_recover() {
    info_.retry_count++;
    info_.state = ConnectionState::RECOVERING;
    // Reset AXI FIFO and resend last packet
    info_.state = ConnectionState::CONNECTED;
    info_.last_error_message = "Recovered: AXI FIFO pipeline reset, transaction retried successfully.";
    return true;
}

void FpgaMockAdapter::update_simulation() {
    if (info_.state == ConnectionState::CONNECTED) {
        cycle_counter_ += 5000;
    }
}

// -------------------------------------------------------------
// JtagMockAdapter (FTDI FT2232H High-Speed USB Adapter)
// -------------------------------------------------------------
JtagMockAdapter::JtagMockAdapter() {
    info_.id = "jtag_ft2232h";
    info_.name = "FTDI FT2232H Dual High-Speed JTAG/SWD Debugger";
    info_.type = DeviceType::JTAG_ADAPTER;
    info_.transport = "USB 2.0 High-Speed (480 Mbps) MPSSE Protocol";
    info_.serial_number = "FTDI-HS-2232-4402A1";
    info_.capabilities = {
        "IEEE_1149_1_BOUNDARY_SCAN",
        "ARM_CORESIGHT_SWD_DEBUG",
        "FPGA_SPI_FLASH_IN_SYSTEM_PROGRAMMING",
        "JTAG_CHAIN_SCAN_AUTODETECTION",
        "PROGRAMMABLE_TCK_CLOCK_100KHZ_TO_30MHZ"
    };
    info_.state = ConnectionState::DISCONNECTED;
    info_.last_error_message = "None";
    info_.retry_count = 0;
}

DeviceInfo JtagMockAdapter::get_info() const {
    return info_;
}

bool JtagMockAdapter::connect() {
    info_.state = ConnectionState::CONNECTED;
    info_.last_error_message = "None";
    return true;
}

bool JtagMockAdapter::disconnect() {
    info_.state = ConnectionState::DISCONNECTED;
    return true;
}

DeviceTelemetry JtagMockAdapter::poll_telemetry() {
    DeviceTelemetry tel;
    tel.timestamp_iso = get_current_time_iso();

    if (info_.state != ConnectionState::CONNECTED) {
        tel.status_metrics["status"] = connection_state_to_string(info_.state);
        tel.status_metrics["error"] = info_.last_error_message;
        return tel;
    }

    tel.numeric_metrics["tck_frequency_kHz"] = 15000.0;
    tel.numeric_metrics["sensed_vref_V"] = vref_sensed_;
    tel.numeric_metrics["scanned_chain_devices"] = 2.0;

    tel.status_metrics["tap_state"] = "RUN_TEST_IDLE";
    tel.status_metrics["device_0_idcode"] = "0x0BA00477 (ARM CoreSight SW-DP)";
    tel.status_metrics["device_1_idcode"] = "0x0362D093 (Xilinx Artix-7 XC7A100T)";
    tel.status_metrics["mpsse_engine_status"] = "STREAMING_NOMINAL";

    return tel;
}

bool JtagMockAdapter::trigger_failure() {
    info_.state = ConnectionState::ERROR_TIMEOUT;
    info_.last_error_message = "Target TDO Parity Error: Expected 0x1 on TMS test transition, sensed floating line. TAP state desynchronized.";
    return true;
}

bool JtagMockAdapter::retry_and_recover() {
    info_.retry_count++;
    info_.state = ConnectionState::RECOVERING;
    // Issue 5 TCK cycles with TMS=1 to force TAP state into Test-Logic-Reset, then transition to Run-Test/Idle
    info_.state = ConnectionState::CONNECTED;
    info_.last_error_message = "Recovered: TAP Test-Logic-Reset sequence pulsed; IDCODE chain verified.";
    return true;
}

void JtagMockAdapter::update_simulation() {
    // Sensed VREF stability check
}

// -------------------------------------------------------------
// EmbeddedManager
// -------------------------------------------------------------
EmbeddedManager::EmbeddedManager() {
    devices_["mcu_stm32h7"] = std::make_shared<McuMockAdapter>();
    devices_["fpga_artix7"] = std::make_shared<FpgaMockAdapter>();
    devices_["jtag_ft2232h"] = std::make_shared<JtagMockAdapter>();
}

std::vector<DeviceInfo> EmbeddedManager::discover_all() {
    std::vector<DeviceInfo> list;
    for (const auto& [id, dev] : devices_) {
        list.push_back(dev->get_info());
    }
    return list;
}

DeviceInfo EmbeddedManager::get_device_info(const std::string& id) {
    auto it = devices_.find(id);
    if (it != devices_.end()) {
        return it->second->get_info();
    }
    DeviceInfo empty;
    empty.id = id;
    empty.state = ConnectionState::DISCONNECTED;
    empty.last_error_message = "Device not found: " + id;
    return empty;
}

bool EmbeddedManager::connect_device(const std::string& id) {
    auto it = devices_.find(id);
    if (it != devices_.end()) {
        return it->second->connect();
    }
    return false;
}

bool EmbeddedManager::disconnect_device(const std::string& id) {
    auto it = devices_.find(id);
    if (it != devices_.end()) {
        return it->second->disconnect();
    }
    return false;
}

DeviceTelemetry EmbeddedManager::get_device_telemetry(const std::string& id) {
    auto it = devices_.find(id);
    if (it != devices_.end()) {
        return it->second->poll_telemetry();
    }
    DeviceTelemetry tel;
    tel.timestamp_iso = get_current_time_iso();
    tel.status_metrics["error"] = "Device not found: " + id;
    return tel;
}

bool EmbeddedManager::trigger_device_failure(const std::string& id) {
    auto it = devices_.find(id);
    if (it != devices_.end()) {
        return it->second->trigger_failure();
    }
    return false;
}

bool EmbeddedManager::recover_device(const std::string& id) {
    auto it = devices_.find(id);
    if (it != devices_.end()) {
        return it->second->retry_and_recover();
    }
    return false;
}

void EmbeddedManager::update_all() {
    for (auto& [id, dev] : devices_) {
        dev->update_simulation();
    }
}

} // namespace workbench
