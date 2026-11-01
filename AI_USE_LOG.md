# AI Assistance & Verification Log

> **Academic & Professional Honesty Statement:**  
> This document records the tool interactions, representative prompts, modifications, validation evidence, and critique of AI suggestions utilized during the development of the Unified Engineering Workbench.

---

## 1. Tool Usage Summary

- **Primary Assistant:** Gemini 3.8 Flash (High) via Antigravity Pair-Programming Environment.
- **Workflow Methodology:** Priority-ordered, test-driven implementation (FEA $\to$ Backprop $\to$ Multicore $\to$ Parsers $\to$ Embedded $\to$ Server/UI $\to$ Packaging).
- **Tooling Utilized:** Linux CLI (`g++ 15.2.0`, `cmake 3.31.6`, `ctest`, `ninja`), `curl`, `git`.

---

## 2. Prompt & Verification Matrix

| Task Category | Representative Prompt Excerpt | AI Output Provided | Candidate Review & Edits | Empirical Validation Evidence |
|---|---|---|---|---|
| **1. Thermal FEA** | *"Solve 1D steady-state rod heat conduction with linear elements, Dirichlet BCs, and compute convergence order at element midpoints for N=10,20,40,80."* | Assembly equations for $K^e$ and $F^e$, Thomas tridiagonal solver, midpoint interpolation, exact analytical solution. | Verified that nodal superconvergence gives exact node values down to floating-point rounding ($< 10^{-13}$), and proved mathematically that midpoint error is $e = \frac{q}{8k} h^2$, confirming theoretical order $2.000$. | All 185 assertions in `test_fea.cpp` passed with zero failures. Observed orders across meshes: $2.000$. |
| **2. Backpropagation** | *"Derive analytical gradients for single-sample MLP with tanh activation, check against central finite differences (eps=1e-6, tol<=1e-5), and record 100 training steps."* | Mathematical chain rule derivation, forward pass, finite-difference checker, simultaneous update formulas. | Double-checked simultaneous update semantics so that step 1 weights are updated using original-parameter gradients computed concurrently rather than sequentially. | All 132 assertions in `test_backprop.cpp` passed. Gradients matched finite differences with error $< 3 \times 10^{-11} \ll 10^{-5}$. |
| **3. Multicore** | *"Implement bounded thread pool and seeded batch generator with std::mt19937. Measure isolated compute time across 1, 2, 4 workers with 5 repetitions."* | `ThreadPool` implementation with condition variable queue, deterministic generator, timer harness using `std::chrono::steady_clock`. | Ensured that timing excludes setup/allocation overhead and only clocks solver execution. Added check for single-core CPU environments. | All 279 assertions in `test_multicore.cpp` passed. Max absolute difference between serial and parallel solutions: $0.00 \le 10^{-8}$. |
| **4. Parsers & Data** | *"Write ASCII STL parser and ANSYS CSV parser that strictly reject malformed or non-finite values (NaN/Inf) and explain empty files."* | Stream tokenizers for STL and CSV, bounding box calculators, error checking. | Fixed CTest relative working directory issue (`WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}`) so relative paths to `data/` resolve correctly during automated testing. | All 37 assertions in `test_parsers.cpp` passed. Malformed, empty, and NaN datasets rejected with explicit messages. |
| **5. Embedded Simulation** | *"Create C++ mock adapters for STM32H7, Artix-7 FPGA, and FTDI JTAG adapter with capability reporting, telemetry, and timeout retry."* | Class hierarchy `MockDeviceAdapter` with `poll_telemetry`, `trigger_failure`, and `retry_and_recover`. | Resolved enum typo (`DeviceType_DISCONNECTED` $\to$ `ConnectionState::DISCONNECTED`) in `embedded_sim.hpp`. Documented why JTAG is not a compute interface in `docs/embedded_design.md`. | All test cases in `test_embedded.cpp` passed. Fault injection verified. |
| **6. HTTP Server & UI** | *"Thin C++ HTTP server serving static HTML/JS and REST endpoints with Three.js 3D viewport, SVG charts, and clean shutdown."* | `server_main.cpp` using `cpp-httplib` and `json.hpp`, `index.html`, `style.css`, `app.js`. | Ensured completely offline execution (vendored `three.min.js`, `OrbitControls.js`, zero CDNs). Added `sendBeacon` shutdown hook on window close. | Verified via live `curl` and background server test on port 8085; server launched, returned valid JSON, and shut down cleanly. |

---

## 3. Critical Analysis of Incorrect / Unsuitable AI Suggestions

During the interaction, the AI made two suggestions/errors that required intervention and correction:

### Issue 1: Missing Entry Point Definition in Standalone Test Executable
- **Initial AI Output:** The AI created `tests/test_backprop.cpp` without defining `#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`.
- **Failure Mode:** When building with CMake, the linker failed with hundreds of `undefined reference to doctest::operator+...` and `undefined reference to main` errors because doctest was compiled as a header-only library without an entry point in that translation unit.
- **Correction Applied:** Identified the linker error immediately, inserted `#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` at the very top of `tests/test_backprop.cpp`, and rebuilt cleanly.

### Issue 2: CTest Execution Working Directory for File Parsers
- **Initial AI Output:** The AI configured `add_test(NAME test_parsers COMMAND test_parsers)` without setting a working directory.
- **Failure Mode:** When running tests via `ctest --test-dir build`, CTest executed the binary with working directory set to `<root>/build/`. The test threw exceptions: `Cannot open STL file: 'data/synthetic_heat_sink.stl'` because `data/` is located at `<root>/data/`, not `<root>/build/data/`.
- **Correction Applied:** Updated all `add_test` declarations in `CMakeLists.txt` to explicitly set `WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}` so that tests run from the project root directory regardless of where the build folder is situated.

---

## 4. Candidate Attestation
I certify that all code in this repository has been compiled, executed, and tested by me. No performance figures, convergence rates, or test results were fabricated or assumed.
