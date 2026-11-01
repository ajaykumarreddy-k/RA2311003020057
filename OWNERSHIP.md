# Intellectual Property & Code Ownership Statement

**Project:** PASS Science Works – Unified Engineering Workbench  
**Candidate Submission**  

This document delineates original work produced for this internship assessment versus external open-source third-party dependencies and their respective licenses.

---

## 1. External Third-Party Dependencies (Permissive Open-Source)

All external libraries are header-only or client-side JavaScript assets, fully vendored in `third_party/` or `web/js/`, requiring **no package managers, no npm, and zero internet access** at build or run time.

| Library | Version | File Location | License | Original Author / Copyright | Purpose |
|---|---|---|---|---|---|
| **cpp-httplib** | v0.18.3 | `third_party/httplib.h` | **MIT License** | Copyright (c) 2024 Yuji Hirose | Single-header C++ HTTP/HTTPS server and static file provider |
| **JSON for Modern C++** | v3.11.3 | `third_party/json.hpp` | **MIT License** | Copyright (c) 2013-2024 Niels Lohmann | Robust C++ JSON serialization/deserialization for REST APIs |
| **doctest** | v2.4.11 | `third_party/doctest.h` | **MIT License** | Copyright (c) 2016-2023 Viktor Kirilov | Lightweight, fast C++ unit testing framework for CTest integration |
| **Three.js** | r128 | `web/js/three.min.js` | **MIT License** | Copyright (c) 2010-2021 Three.js Authors | Client-side WebGL 3D rendering engine |
| **OrbitControls** | r128 | `web/js/OrbitControls.js` | **MIT License** | Copyright (c) 2010-2021 Three.js Authors | Camera orbit, zoom, and pan controls for 3D viewport |

---

## 2. Original Implementation & Source Code

All components listed below were authored specifically for this project:

### 2.1 C++ Core Numerics, Parsers & Simulation (`src/core/`)
- `fea.hpp`, `fea.cpp`:
  - 1D steady-state finite element thermal conduction formulation.
  - Linear 2-node stiffness matrix assembly ($K^e = \frac{kA}{h} [1, -1; -1, 1]$) and uniform source vector assembly ($F^e = \frac{qAh}{2} [1; 1]$).
  - Exact Dirichlet boundary condition symmetric modification and Thomas algorithm tridiagonal solver.
  - Element midpoint linear interpolation, exact analytical benchmark comparison, and convergence order calculator ($O(h^2)$).
  - Strict parameter validation ($L > 0, A > 0, k > 0, N \ge 1$).
- `backprop.hpp`, `backprop.cpp`:
  - Single-sample neural network forward pass: $z = wx+b$, $h = \tanh(z)$, $\hat{y} = vh+c$, $J = \frac{1}{2}(\hat{y}-y)^2$.
  - Explicit analytic chain-rule gradients for $w, b, v, c$.
  - Central finite difference checker ($g_{\text{fd}} = \frac{J(\theta+\epsilon)-J(\theta-\epsilon)}{2\epsilon}$) verifying $|g_{\text{analytic}} - g_{\text{fd}}| \le 10^{-5}$.
  - Simultaneous first update and 100-step training loop with loss telemetry.
- `thread_pool.hpp`:
  - Bounded multi-worker thread pool built exclusively on `std::thread`, `std::mutex`, and `std::condition_variable`.
- `multicore_benchmark.hpp`, `multicore_benchmark.cpp`:
  - Deterministic pseudo-random batch generation via `std::mt19937` with fixed seed.
  - Serial vs parallel execution with input-order preservation.
  - Rigorous solution verification ($\max |T_{\text{serial}} - T_{\text{parallel}}| \le 10^{-8}$).
  - Isolated compute timing harness (`std::chrono::steady_clock`) across $\{1, 2, 4\}$ threads with 1 warmup and 5 measured repetitions, reporting median time, speedup $T_1/T_p$, and efficiency $S_p/p$.
  - Host CPU core count, compiler, and optimization flag introspection.
- `stl_parser.hpp`, `stl_parser.cpp`:
  - Validated ASCII STL parser with case-insensitive tokenization.
  - Explicit error handling for empty files, whitespace-only files, non-finite floats (`NaN`/`Inf`), incomplete loops, and bounding box computation.
- `ansys_parser.hpp`, `ansys_parser.cpp`:
  - Strict ANSYS CSV nodal parser (`node_id,x_m,y_m,z_m,temperature_C`).
  - Validation against missing columns, malformed integer IDs, non-finite values, and empty data sets.
- `embedded_sim.hpp`, `embedded_sim.cpp`:
  - Mock adapters for STM32H7 MCU (USB-UART), Artix-7 FPGA (SPI/PCIe), and FTDI FT2232H JTAG adapter.
  - Device discovery, capability reporting, live telemetry generation, and simulated timeout/CRC/parity faults with recovery.

### 2.2 Server, Launcher & Background Jobs (`src/server/`, `src/launcher/`)
- `server_main.cpp`: Thin REST HTTP API endpoints and static file server using `cpp-httplib`.
- `job_manager.hpp`: Thread-safe asynchronous job scheduler supporting live polling and non-blocking job cancellation.
- `launcher_main.cpp`: Portable launcher that detects an open localhost port, spawns the server as a silent background process without console popups, opens the default web browser, and cleanly handles termination.

### 2.3 Web Presentation & Offline Styling (`web/`)
- `index.html`: Responsive single-page workbench with four dedicated tabs (Geometry, Thermal, Performance, Hardware).
- `css/style.css`: Dark slate engineering UI with modern typography, telemetry metric cards, and responsive tables.
- `web/js/app.js`: Tab switching, Three.js 3D viewport with temperature colormap and raycaster node inspection, SVG chart renderers, background benchmark polling, and hardware fault injection controls.

### 2.4 Test Suite & Synthetic Assets (`tests/`, `data/`)
- 5 comprehensive CTest test suites (`test_fea.cpp`, `test_backprop.cpp`, `test_multicore.cpp`, `test_parsers.cpp`, `test_embedded.cpp`).
- `data/synthetic_heat_sink.stl` & `data/synthetic_ansys_thermal.csv`: Procedurally generated geometric models accompanied by `data/PROVENANCE.md`.

### 2.5 Documentation (`docs/`, root)
- `docs/embedded_design.md`: Interface specification, pinout table, voltage domains, level shifting, and JTAG rationale.
- `docs/report.md`: Technical evaluation report.
- `docs/handwritten_backprop_notes.md`: 2-page handwritten preparation guide.
- `README.md`, `AI_USE_LOG.md`, `OWNERSHIP.md`.
