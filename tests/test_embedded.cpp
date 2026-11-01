#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "embedded_sim.hpp"

using namespace workbench;

TEST_CASE("Task 5 Embedded: Device Discovery and Capability Reporting") {
    EmbeddedManager mgr;
    auto devices = mgr.discover_all();

    REQUIRE(devices.size() == 3);

    bool found_mcu = false;
    bool found_fpga = false;
    bool found_jtag = false;

    for (const auto& dev : devices) {
        if (dev.type == DeviceType::MICROCONTROLLER) {
            found_mcu = true;
            CHECK(dev.id == "mcu_stm32h7");
            CHECK(!dev.capabilities.empty());
            CHECK(dev.state == ConnectionState::DISCONNECTED);
        } else if (dev.type == DeviceType::FPGA) {
            found_fpga = true;
            CHECK(dev.id == "fpga_artix7");
            CHECK(!dev.capabilities.empty());
            CHECK(dev.state == ConnectionState::DISCONNECTED);
        } else if (dev.type == DeviceType::JTAG_ADAPTER) {
            found_jtag = true;
            CHECK(dev.id == "jtag_ft2232h");
            CHECK(!dev.capabilities.empty());
            CHECK(dev.state == ConnectionState::DISCONNECTED);
        }
    }

    CHECK(found_mcu);
    CHECK(found_fpga);
    CHECK(found_jtag);
}

TEST_CASE("Task 5 Embedded: Connect, Disconnect, and Telemetry Polling") {
    EmbeddedManager mgr;

    // Test MCU
    CHECK(mgr.connect_device("mcu_stm32h7"));
    auto mcu_info = mgr.get_device_info("mcu_stm32h7");
    CHECK(mcu_info.state == ConnectionState::CONNECTED);

    auto mcu_tel = mgr.get_device_telemetry("mcu_stm32h7");
    CHECK(!mcu_tel.timestamp_iso.empty());
    CHECK(mcu_tel.numeric_metrics.count("core_temperature_C") > 0);
    CHECK(mcu_tel.numeric_metrics["core_temperature_C"] > 20.0);
    CHECK(mcu_tel.numeric_metrics["vcc_core_V"] == doctest::Approx(1.201).epsilon(1e-3));
    CHECK(mcu_tel.status_metrics.count("firmware_version") > 0);

    CHECK(mgr.disconnect_device("mcu_stm32h7"));
    mcu_info = mgr.get_device_info("mcu_stm32h7");
    CHECK(mcu_info.state == ConnectionState::DISCONNECTED);

    // Test FPGA
    CHECK(mgr.connect_device("fpga_artix7"));
    auto fpga_tel = mgr.get_device_telemetry("fpga_artix7");
    CHECK(fpga_tel.numeric_metrics.count("die_temperature_C") > 0);
    CHECK(fpga_tel.numeric_metrics.count("lut_utilization_pct") > 0);
    CHECK(fpga_tel.status_metrics.count("bitstream_id") > 0);

    // Test JTAG
    CHECK(mgr.connect_device("jtag_ft2232h"));
    auto jtag_tel = mgr.get_device_telemetry("jtag_ft2232h");
    CHECK(jtag_tel.numeric_metrics.count("tck_frequency_kHz") > 0);
    CHECK(jtag_tel.status_metrics.count("tap_state") > 0);
    CHECK(jtag_tel.status_metrics["tap_state"] == "RUN_TEST_IDLE");
}

TEST_CASE("Task 5 Embedded: Recoverable Timeout Failure and Auto-Recovery") {
    EmbeddedManager mgr;

    // Connect MCU
    mgr.connect_device("mcu_stm32h7");
    CHECK(mgr.get_device_info("mcu_stm32h7").state == ConnectionState::CONNECTED);

    // Trigger recoverable failure
    CHECK(mgr.trigger_device_failure("mcu_stm32h7"));
    auto fail_info = mgr.get_device_info("mcu_stm32h7");
    CHECK(fail_info.state == ConnectionState::ERROR_TIMEOUT);
    CHECK(!fail_info.last_error_message.empty());
    CHECK(fail_info.last_error_message.find("Timeout") != std::string::npos);

    // Telemetry shows error
    auto err_tel = mgr.get_device_telemetry("mcu_stm32h7");
    CHECK(err_tel.status_metrics["status"] == "ERROR_TIMEOUT");

    // Recover device
    CHECK(mgr.recover_device("mcu_stm32h7"));
    auto rec_info = mgr.get_device_info("mcu_stm32h7");
    CHECK(rec_info.state == ConnectionState::CONNECTED);
    CHECK(rec_info.retry_count == 1);
    CHECK(rec_info.last_error_message.find("Recovered") != std::string::npos);

    // Telemetry operational again
    auto ok_tel = mgr.get_device_telemetry("mcu_stm32h7");
    CHECK(ok_tel.numeric_metrics.count("core_temperature_C") > 0);
}
