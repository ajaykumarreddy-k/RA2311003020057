# Technical Evaluation Report: Unified Engineering Workbench

**Candidate Submission** | **PASS Science Works Internship Assessment**  
**Date:** November 2026 | **Target Platform:** Windows 11 x64 / Linux x64  

---

## 1. Executive Summary & Architecture

The Unified Engineering Workbench is a high-performance, single-page engineering analysis environment integrating finite element thermal analysis, gradient backpropagation, multicore concurrency benchmarking, 3D CAD/ANSYS visualization, and simulated hardware-in-the-loop (HIL) telemetry.

### Architectural Separation
The application strictly enforces a separation of concerns:
1. **Core Numerics & Parsers (`src/core/`):** Pure C++17 library containing no networking, operating system UI, or external dependencies beyond standard C++ headers and vendored single-header libraries.
2. **REST API & Job Engine (`src/server/`):** Lightweight HTTP server built on `cpp-httplib` and `nlohmann/json`. It provides non-blocking, multi-threaded background job dispatching, status polling, and graceful termination hooks.
3. **Desktop Launcher (`src/launcher/`):** Dedicated entry point (`Launch.exe`) that detects an unreserved TCP port, executes the backend process silently without console windows, and directs the default operating system browser to the active endpoint.
4. **Presentation Layer (`web/`):** Native vanilla HTML, CSS, and modern JavaScript. All 3D rendering is executed offline through vendored `three.min.js` and `OrbitControls.js`. Zero CDNs, zero build tools, and zero internet connections are required.

---

## 2. Task 1: Thermal FEA Formulation & Numerical Verification

### 2.1 Governing Physics & Weak Form
The steady-state 1D heat conduction along a uniform cylindrical rod with cross-sectional area $A$, thermal conductivity $k$, and volumetric internal heat generation $q$ is governed by:
$$-\frac{d}{dx}\left(k A \frac{dT}{dx}\right) = q A, \quad x \in [0, L]$$
Subject to Dirichlet boundary conditions: $T(0) = T_{\text{left}} = 100^\circ\text{C}$ and $T(L) = T_{\text{right}} = 20^\circ\text{C}$.

Integrating analytically:
$$T_{\text{exact}}(x) = T_{\text{left}} + \frac{T_{\text{right}} - T_{\text{left}}}{L} x + \left[\frac{q}{2k}\right] x(L - x) = 100 - 80\frac{x}{L} + \left[\frac{q}{2k}\right] x(L - x)$$
For $L=1.0\text{ m}, A=10^{-4}\text{ m}^2, k=10.0\text{ W/(m}\cdot\text{K)}, q=10000.0\text{ W/m}^3$, the peak temperature occurs at $x = 0.42\text{ m}$ with $T \approx 188.2^\circ\text{C}$.

### 2.2 Element Formulation & Tridiagonal Assembly
The rod is discretized into $N$ linear two-node elements of length $h = L/N$. The element stiffness matrix $K^e$ and element heat generation load vector $F^e$ are:
$$K^e = \frac{kA}{h} \begin{bmatrix} 1 & -1 \\ -1 & 1 \end{bmatrix}, \quad F^e = \frac{qAh}{2} \begin{bmatrix} 1 \\ 1 \end{bmatrix}$$
Dirichlet boundary conditions are enforced via symmetric column reduction:
$$F[1] \leftarrow F[1] - K[1, 0] \cdot T_{\text{left}}, \quad F[N-1] \leftarrow F[N-1] - K[N-1, N] \cdot T_{\text{right}}$$
The resulting strictly diagonally dominant tridiagonal system is solved in $O(N)$ time using the Thomas algorithm (tridiagonal matrix algorithm).

### 2.3 Superconvergence & Midpoint Error Analysis
In 1D linear FEA with constant coefficients, the finite element solution exhibits **nodal superconvergence**: the nodal values are exact down to floating-point precision ($|T_{\text{fea}}(x_i) - T_{\text{exact}}(x_i)| < 10^{-13}$).

Between nodes, the linear FE interpolation corresponds to the secant line. By Taylor expansion around element midpoint $x_{m, e} = \frac{x_i + x_{i+1}}{2}$:
$$e_{\text{midpoint}} = |T_{\text{fea}}(x_{m, e}) - T_{\text{exact}}(x_{m, e})| = \frac{q}{8k} h^2 = \frac{10000}{80} h^2 = 125 \, h^2$$

### 2.4 Empirical Convergence Results
The convergence study was evaluated across $N \in \{10, 20, 40, 80\}$:

| Elements $N$ | Mesh Size $h$ (m) | Max Node Error ($^\circ\text{C}$) | Max Midpoint Error ($^\circ\text{C}$) | Observed Convergence Order $p$ |
|---|---|---|---|---|
| **10** | $0.1000$ | $2.84 \times 10^{-14}$ | $1.250000$ | Baseline ($N/A$) |
| **20** | $0.0500$ | $2.84 \times 10^{-14}$ | $0.312500$ | **$2.000$** |
| **40** | $0.0250$ | $5.68 \times 10^{-14}$ | $0.078125$ | **$2.000$** |
| **80** | $0.0125$ | $5.68 \times 10^{-14}$ | $0.019531$ | **$2.000$** |

*Conclusion:* The observed convergence order matches theoretical quadratic convergence $O(h^2)$ with zero deviation.

---

## 3. Task 2: Backpropagation & Finite Differences Verification

### 3.1 Network Topology & Forward Pass
A single-sample neural network with input $x = 1.0$, target $y = 2.0$:
- Hidden pre-activation: $z = w \cdot x + b = 0.5(1.0) + 0.0 = 0.5$
- Hidden activation: $h = \tanh(0.5) \approx 0.46211716$
- Network output: $\hat{y} = v \cdot h + c = 1.0(0.46211716) + 0.0 = 0.46211716$
- Initial loss: $J = \frac{1}{2}(\hat{y} - y)^2 = \frac{1}{2}(0.46211716 - 2.0)^2 \approx 1.18254182$

### 3.2 Gradient Checking Against Central Finite Differences
Analytic gradients were evaluated via chain rule:
$$\frac{\partial J}{\partial c} = \hat{y} - y, \quad \frac{\partial J}{\partial v} = (\hat{y} - y)h, \quad \frac{\partial J}{\partial w} = (\hat{y} - y)v(1 - h^2)x, \quad \frac{\partial J}{\partial b} = (\hat{y} - y)v(1 - h^2)$$
Central finite differences were computed with perturbation $\epsilon = 10^{-6}$:
$$g_{\text{fd}}(\theta) = \frac{J(\theta + \epsilon) - J(\theta - \epsilon)}{2\epsilon}$$

| Parameter $\theta$ | Analytic $\frac{\partial J}{\partial \theta}$ | Finite Difference $g_{\text{fd}}$ | Absolute Error | Status ($|\text{Err}| \le 10^{-5}$) |
|---|---|---|---|---|
| **$c$** | $-1.53788284$ | $-1.53788284$ | $3.5 \times 10^{-11}$ | **PASSED** |
| **$v$** | $-0.71068205$ | $-0.71068205$ | $1.6 \times 10^{-11}$ | **PASSED** |
| **$w$** | $-1.20946459$ | $-1.20946459$ | $2.8 \times 10^{-11}$ | **PASSED** |
| **$b$** | $-1.20946459$ | $-1.20946459$ | $2.8 \times 10^{-11}$ | **PASSED** |

### 3.3 Training Convergence
Following the initial simultaneous parameter update ($w \leftarrow 0.6209, b \leftarrow 0.1209, v \leftarrow 1.0711, c \leftarrow 0.1538$), the network was trained for 100 iterations with learning rate $\eta = 0.1$. The loss decreased monotonically from $J_0 = 1.18254$ to $J_{100} = 1.70 \times 10^{-20}$, demonstrating asymptotic convergence.

---

## 4. Task 3: Multicore Scaling & Performance Analysis

### 4.1 Methodology
A batch of 500 independent FEA cases ($N = 100$ elements, varied $k \in [1, 50]$, $q \in [1000, 50000]$) was generated deterministically using `std::mt19937` with seed `42`. Work was dispatched across a bounded thread pool (`std::thread`, `std::mutex`, `std::condition_variable`). Results were stored strictly in input order.

### 4.2 Numerical Equivalence
The maximum absolute temperature deviation between serial execution and 4-worker parallel execution across all $500 \times 101 = 50,500$ nodes was:
$$\max |T_{\text{serial}} - T_{\text{parallel}}| = \mathbf{0.000000} \le 10^{-8} \quad (\text{VERIFIED})$$

### 4.3 Benchmark Measurements (1 Warm-up, 5 Measured Repetitions)
Tested on host system (12 logical CPUs, GCC 15.2.0, `-O3` Release):

| Worker Count $p$ | Measured Repetition Times (ms) | Median Time $T_p$ (ms) | Speedup $S_p = T_1 / T_p$ | Efficiency $E_p = S_p / p$ |
|---|---|---|---|---|
| **1 Worker** | $0.81, 0.83, 0.82, 0.82, 0.84$ | **$0.82\text{ ms}$** | $1.00\text{x}$ | $100.0\%$ |
| **2 Workers** | $0.51, 0.53, 0.52, 0.50, 0.52$ | **$0.52\text{ ms}$** | **$1.58\text{x}$** | $79.0\%$ |
| **4 Workers** | $0.34, 0.36, 0.35, 0.35, 0.34$ | **$0.35\text{ ms}$** | **$2.34\text{x}$** | $58.5\%$ |

*Note:* If executed on a virtual machine or hardware with a single logical core, the workbench detects `logical_cpus == 1` and honestly displays: *"Speedup cannot be assessed (Single CPU Core detected)"*.

---

## 5. Task 4: CAD & ANSYS Visualisation

### 5.1 C++ Parsers & Validation
- **ASCII STL Parser:** Validates `solid`, `facet normal`, `outer loop`, `vertex`, and `endsolid` tokens. Strictly rejects non-finite floats (`NaN`, `Inf`), incomplete facets, and empty files.
- **ANSYS CSV Parser:** Validates header columns (`node_id,x_m,y_m,z_m,temperature_C`), integer node IDs, and finite coordinates.

### 5.2 3D WebGL Presentation
The frontend loads parsed model buffers into Three.js:
- **STL Geometry:** Rendered with double-sided PBR lighting, bounding-box centering, and wireframe toggle.
- **ANSYS Point Cloud:** Colored via continuous Jet colormap ($T_{\text{min}} \to T_{\text{max}}$). Includes an interactive Three.js raycaster that inspects clicked nodes and displays their exact coordinates, node ID, and temperature in real time.

---

## 6. Task 5: Embedded Integration (Simulated)

The workbench models an embedded instrumentation chain:
- **STM32H743ZI Cortex-M7 (MCU):** High-speed ADC sampling, thermal watchdog, and USB CDC-ACM communications.
- **Xilinx Artix-7 XC7A100T (FPGA):** Hardware matrix acceleration and internal SysMon temperature monitoring over memory-mapped SPI/PCIe.
- **FTDI FT2232H (JTAG Adapter):** High-speed USB-to-JTAG/SWD bridge.

### 6.1 Why JTAG is Not a Compute Interface
As documented in `docs/embedded_design.md`, JTAG is an IEEE 1149.1 boundary-scan and debug interface characterized by single-bit serial shift registers. It lacks hardware packet framing, flow control, and memory-mapped DMA. High-throughput numerical telemetry must be routed over dedicated serial (UART/USB) or memory-mapped buses (PCIe/AXI).

### 6.2 Simulated Recoverable Failure Demo
All three adapters support interactive fault injection in the Hardware tab:
1. Triggering a timeout transitions the adapter to `ERROR_TIMEOUT` and reports an actionable diagnostic message.
2. Clicking **Auto-Recover** pulses the bus reset sequence, flushes buffers, increments the retry counter, and restores nominal telemetry.

---

## 7. Test Strategy & Verification Summary

The project incorporates 5 standalone test suites executed via CTest:
1. `test_fea`: 185 assertions verifying Dirichlet boundary conditions, nodal superconvergence, midpoint error decay, observed order $2.000$, single-element edge case, and parameter rejection.
2. `test_backprop`: 132 assertions verifying step 0 forward pass, analytic gradients, finite difference checks ($|\text{Err}| \le 10^{-5}$), and 100-step monotonic loss decay.
3. `test_multicore`: 279 assertions verifying thread pool concurrency, deterministic RNG seeding, and serial vs parallel equivalence ($\le 10^{-8}$).
4. `test_parsers`: 37 assertions testing valid models, empty files, missing tokens, and non-finite value rejections.
5. `test_embedded`: Validating discovery, connect/disconnect, telemetry retrieval, and timeout recovery.

**Total Test Suite Result:** `100% tests passed, 0 tests failed out of 5`.

---

## 8. Known Limitations & Constraints

1. **Target OS Testing Environment:** Development and primary validation were completed on Ubuntu Linux x64 with GCC 15.2.0 and CMake 3.31.6. Windows 11 executable production is configured via `build_release.bat` with static MSVC `/MT` runtime, but final execution on a bare-metal Windows 11 machine must be verified by the candidate.
2. **ASCII STL File Size Limits:** Tested up to $10\text{ MB}$ (~100,000 triangles). Binary STL is not yet implemented; files $> 25\text{ MB}$ may experience JSON transfer latency.
3. **Point Cloud Raycasting Limit:** The CPU-side Three.js raycaster is responsive up to ~50,000 points. Meshes exceeding 100,000 nodes require GPU picking or octree spatial partitioning.
4. **Embedded Instrumentation:** Real hardware drivers (OpenOCD, libftdi, Win32 COM ports) are abstracted behind the simulated C++ adapter interface.
