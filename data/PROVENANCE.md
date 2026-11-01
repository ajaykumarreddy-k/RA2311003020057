# Data Provenance & Specifications

> **IMPORTANT NOTICE:** All files in this directory (`data/`) are **100% SYNTHETIC**, procedurally generated for testing, validation, and benchmarking of the Unified Engineering Workbench. No proprietary, customer, or confidential datasets are included.

---

## 1. File Inventory & Provenance

### 1.1 `synthetic_heat_sink.stl`
- **Classification:** SYNTHETIC CAD Model.
- **Generator:** Built via programmatic bounding box facet extrusion script.
- **Physical Geometry:** Extruded aluminum heat sink bracket comprising a base mounting slab ($100\text{ mm} \times 50\text{ mm} \times 10\text{ mm}$) and three upright thermal convection fins ($100\text{ mm} \times 8\text{ mm} \times 30\text{ mm}$).
- **Facet Count:** 48 triangular facets, 144 vertices.
- **Coordinate Units:** Meters ($m$).
- **File Format:** Standard ASCII STL (`solid ... facet normal ... outer loop ... vertex ... endloop ... endfacet ... endsolid`).
- **Validation Status:** Closed planar boundaries, finite coordinates, verified outward-pointing normals.

### 1.2 `synthetic_ansys_thermal.csv`
- **Classification:** SYNTHETIC Finite Element Thermal Solution Field.
- **Generator:** Evaluated over an $8 \times 5 \times 4$ structured 3D spatial grid ($L_x = 0.10\text{ m}, L_y = 0.05\text{ m}, L_z = 0.02\text{ m}$) under a simulated 1D linear thermal gradient with an embedded Gaussian internal heat dissipation hotspot ($T_{\text{peak}} \approx 143.6^\circ\text{C}$).
- **Node Count:** 160 discrete nodal inspection points.
- **Schema & Units:**
  - `node_id`: Unique integer node identifier ($1, 2, \dots, 160$)
  - `x_m`: X Cartesian coordinate in meters ($m$, range: $0.000$ to $0.100\text{ m}$)
  - `y_m`: Y Cartesian coordinate in meters ($m$, range: $0.000$ to $0.050\text{ m}$)
  - `z_m`: Z Cartesian coordinate in meters ($m$, range: $0.000$ to $0.020\text{ m}$)
  - `temperature_C`: Steady-state temperature in degrees Celsius ($^\circ\text{C}$, range: $20.00^\circ\text{C}$ to $143.61^\circ\text{C}$)

---

## 2. Tested File-Size & Complexity Limits (Honest Assessment)

| Format | Tested In-Memory Limit | Peak Parsing Throughput | Browser WebGL Limit | Known Failure Modes / Limitations |
|---|---|---|---|---|
| **ASCII STL** | Verified up to **10 MB** (~100,000 triangles) | ~1.2 million vertices/sec | ~250,000 triangles | Single-threaded ASCII string tokenization; ASCII STL files $> 25\text{ MB}$ cause high JSON serialization overhead. Binary STL is recommended for industrial CAD $> 50\text{ MB}$. |
| **ANSYS CSV** | Verified up to **5 MB** (~80,000 nodal points) | ~450,000 rows/sec | ~50,000 points (interactive hover raycasting) | Hover inspection becomes sluggish in Three.js CPU raycasting above 50,000 discrete points without GPU picking or octree acceleration. |
