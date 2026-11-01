# Unified Engineering Workbench (PASS Science Works)

> **High-Performance Engineering Workbench**  
> Built with pure **C++17**, an asynchronous HTTP REST service, and an offline **HTML/JS Three.js** presentation layer. Statically linked for standalone execution without runtime DLL dependencies, admin rights, or internet access.

---

## 1. How to Run (Quick Start & One-Click Launch)

### Option A: Standalone Packaged Run (Zero Installs, Zero Terminal)
- **Windows 11 x64:**
  1. Open the `dist/` directory.
  2. Double-click **`Launch.exe`**.
  3. The launcher will automatically find an available localhost port, start `workbench_server.exe` silently in the background (no console window or PowerShell popup), and open your default browser to `http://127.0.0.1:<port>/`.
  4. Closing the browser tab or closing the launcher automatically shuts down the server.

- **Linux x64:**
  ```bash
  ./dist/Launch
  ```
  *(Finds a free port, starts the server in background, and opens your default browser via `xdg-open`)*.

### Option B: Run from Source / Build Directory
```bash
# 1. Configure and Build (Release mode)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 2. Run All Automated Unit Tests (doctest / CTest)
ctest --test-dir build --output-on-failure

# 3. Run via Launcher
./build/Launch

# Or run the HTTP backend server directly:
./build/workbench_server --port 8080
# Then open http://127.0.0.1:8080/ in your browser
```

### Option C: VS Code One-Action Run (F5)
1. Open this repository folder in VS Code.
2. Press **`F5`** (or go to **Run and Debug** $\to$ select **"Launch Workbench (Packaged)"** $\to$ click Play).
3. VS Code will automatically run the `Build All` task and launch the workbench.

### Clean Shutdown Behavior
- Closing the browser tab sends a clean shutdown beacon (`/api/shutdown`) to terminate the backend server.
- Terminating `Launch` (or pressing `Ctrl+C` in terminal mode) signals the server process to cleanly flush and exit.

---

## 2. Architecture & Folder Structure

```
.
├── CMakeLists.txt                # Root CMake build configuration (CTest enabled, static runtime)
├── build_release.bat             # Automated Windows 11 MSVC /MT Release build & dist/ packager
├── OWNERSHIP.md                  # Intellectual property statement & open-source licenses
├── AI_USE_LOG.md                 # Complete AI prompts, validation evidence, & critique log
├── README.md                     # Project documentation & quickstart guide
├── .gitignore                    # Build artifacts and temporary file exclusions
│
├── src/
│   ├── core/                     # Pure C++17 Numerics, Parsers & Hardware Adapters (No UI/Net)
│   │   ├── fea.hpp / .cpp        # 1D steady-state Thermal FEA, Thomas solver, convergence study
│   │   ├── backprop.hpp / .cpp   # Analytic backprop, finite difference checker, training loop
│   │   ├── thread_pool.hpp       # Bounded worker thread pool (std::thread, mutex, condition_var)
│   │   ├── multicore_benchmark.hpp/.cpp # Seeded batch generator, verification & timing harness
│   │   ├── stl_parser.hpp / .cpp # Validated ASCII STL parser with bounding box computation
│   │   ├── ansys_parser.hpp/.cpp # Validated ANSYS CSV nodal parser (temperature field)
│   │   └── embedded_sim.hpp/.cpp # Simulated MCU, FPGA, and JTAG mock adapters
│   │
│   ├── server/                   # Thin C++ HTTP Server (cpp-httplib & nlohmann/json)
│   │   ├── server_main.cpp       # REST API endpoints & static web asset provider
│   │   └── job_manager.hpp       # Asynchronous background job manager with polling & cancel
│   │
│   └── launcher/                 # Silent Application Launcher (Launch.exe)
│       └── launcher_main.cpp     # Detects free port, spawns server silently, opens default browser
│
├── web/                          # Offline Presentation Layer (Zero CDN, Zero npm/build steps)
│   ├── index.html                # Single-page workbench (Geometry, Thermal, Performance, Hardware)
│   ├── css/style.css             # Dark slate engineering UI, responsive layouts, data tables
│   └── js/
│       ├── app.js                # Tab switching, Three.js 3D viewport, SVG charts, job polling
│       ├── three.min.js          # Vendored Three.js (r128, MIT License)
│       └── OrbitControls.js      # Vendored OrbitControls (r128, MIT License)
│
├── data/                         # Synthetic Test Datasets
│   ├── synthetic_heat_sink.stl   # Synthetic 48-facet ASCII STL bracket
│   ├── synthetic_ansys_thermal.csv # Synthetic 160-node ANSYS 3D thermal grid
│   └── PROVENANCE.md             # Data provenance notes and tested file-size limits
│
├── docs/                         # Technical Documentation
│   ├── report.md                 # Technical evaluation report (FEA, backprop, multicore, tests)
│   ├── embedded_design.md        # Interface & pinout specification, Mermaid diagram, JTAG analysis
│   └── handwritten_backprop_notes.md # 2-page handwritten backpropagation guide for candidate
│
├── tests/                        # Automated Unit Tests (doctest v2.4.11)
│   ├── test_fea.cpp              # FEA BCs, nodal superconvergence, midpoint order O(h^2)
│   ├── test_backprop.cpp         # Forward pass, analytic vs FD gradients (tol <= 1e-5), training
│   ├── test_multicore.cpp        # Thread pool concurrency, deterministic RNG, serial vs parallel
│   ├── test_parsers.cpp          # STL and ANSYS parsers, malformed / NaN / empty file rejection
│   └── test_embedded.cpp         # Device discovery, capabilities, telemetry, timeout auto-recovery
│
├── dist/                         # Packaged Standalone Windows/Linux Deployment Directory
└── third_party/                  # Vendored Single-Header Libraries (MIT License)
    ├── doctest.h                 # doctest single-header testing framework
    ├── httplib.h                 # cpp-httplib single-header HTTP server
    └── json.hpp                  # nlohmann/json single-header JSON parser
```

---

## 3. Supported File Formats & Validation

1. **ASCII STL (`.stl`):**
   - Supports `solid ... facet normal ... outer loop ... vertex ... endloop ... endfacet ... endsolid`.
   - Rejects non-finite floats (`NaN`, `Inf`), incomplete loops, and whitespace-only files with actionable error messages.
2. **ANSYS Nodal CSV (`.csv`):**
   - Header: `node_id,x_m,y_m,z_m,temperature_C`.
   - Validates integer node IDs, floating-point coordinates, and temperature bounds.

---

## 4. Verification & Test Execution

Run the complete test suite across all 5 modules:
```bash
ctest --test-dir build --output-on-failure
```
**Outcome:** 100% tests passed (0 failures out of 5 test suites).

---

## 5. Building for Production Release

### Windows 11 x64
Open **Developer Command Prompt for VS** or **x64 Native Tools Command Prompt** and run:
```bat
build_release.bat
```
This executes CMake with static runtime linking (`/MT`), runs all unit tests via CTest, and stages the self-contained package into `dist/`.

---

## 6. Known Limitations (Honest Disclosure)
1. **Host Testing Environment:** Developed and tested on Ubuntu Linux x64 with GCC 15.2.0. The Windows 11 build scripts (`build_release.bat`) configure static MSVC `/MT` compilation, but execution on a physical Windows 11 machine must be performed using the provided script.
2. **ASCII STL Parsing Throughput:** Tested up to 10 MB (~100,000 triangles). Binary STL is recommended for models $> 25\text{ MB}$.
3. **Point Cloud Raycasting Limit:** The CPU-based Three.js raycaster is responsive up to ~50,000 points.
4. **Hardware Interfaces:** Embedded hardware interactions are simulated via C++ mock adapters (`workbench::EmbeddedManager`).
