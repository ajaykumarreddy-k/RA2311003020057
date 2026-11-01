// Unified Engineering Workbench Frontend Controller

document.addEventListener('DOMContentLoaded', () => {
    initTabs();
    initThreeJsViewport();
    initThermalFea();
    initBackpropagation();
    initPerformanceBenchmark();
    initHardwareSim();
    checkBackendHealth();

    // Clean server shutdown when tab is closed
    window.addEventListener('beforeunload', () => {
        try {
            navigator.sendBeacon('/api/shutdown');
        } catch (e) {
            // Best effort shutdown
        }
    });
});

// -------------------------------------------------------------
// 1. Tab Navigation
// -------------------------------------------------------------
function initTabs() {
    const tabBtns = document.querySelectorAll('.tab-btn');
    const panes = document.querySelectorAll('.tab-pane');

    tabBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            const targetId = btn.getAttribute('data-tab');

            tabBtns.forEach(b => b.classList.remove('active'));
            panes.forEach(p => p.classList.remove('active'));

            btn.classList.add('active');
            const targetPane = document.getElementById(targetId);
            if (targetPane) {
                targetPane.classList.add('active');
            }

            // Trigger window resize so Three.js canvas resizes correctly
            if (targetId === 'geometry-tab') {
                window.dispatchEvent(new Event('resize'));
            }
        });
    });
}

function checkBackendHealth() {
    fetch('/api/health')
        .then(r => r.json())
        .then(data => {
            const dot = document.getElementById('server-status-dot');
            const text = document.getElementById('server-status-text');
            if (data.status === 'ok') {
                dot.className = 'status-indicator online';
                text.textContent = 'Backend Online (C++17)';
            }
        })
        .catch(() => {
            const dot = document.getElementById('server-status-dot');
            const text = document.getElementById('server-status-text');
            dot.className = 'status-indicator';
            text.textContent = 'Backend Offline';
        });
}

// -------------------------------------------------------------
// 2. Geometry & 3D Three.js Viewport
// -------------------------------------------------------------
let threeScene, threeCamera, threeRenderer, threeControls;
let currentMesh = null;
let currentPointCloud = null;
let pointCloudData = null; // { positions, temperatures, node_ids, min_temp, max_temp }
let wireframeActive = false;

function initThreeJsViewport() {
    const container = document.getElementById('threejs-canvas-wrapper');
    if (!container) return;

    threeScene = new THREE.Scene();
    threeScene.background = new THREE.Color(0x0f141c);

    // Camera
    threeCamera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.001, 100);
    threeCamera.position.set(0.15, 0.15, 0.2);

    // Renderer
    threeRenderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    threeRenderer.setSize(container.clientWidth, container.clientHeight);
    threeRenderer.setPixelRatio(window.devicePixelRatio);
    container.appendChild(threeRenderer.domElement);

    // OrbitControls
    threeControls = new THREE.OrbitControls(threeCamera, threeRenderer.domElement);
    threeControls.enableDamping = true;
    threeControls.dampingFactor = 0.05;

    // Lighting
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.7);
    threeScene.add(ambientLight);

    const dirLight1 = new THREE.DirectionalLight(0xffffff, 0.8);
    dirLight1.position.set(1, 2, 3);
    threeScene.add(dirLight1);

    const dirLight2 = new THREE.DirectionalLight(0x38bdf8, 0.4);
    dirLight2.position.set(-1, -2, -1);
    threeScene.add(dirLight2);

    // Grid helper
    const grid = new THREE.GridHelper(0.3, 30, 0x2b394f, 0x1e293b);
    grid.position.y = -0.001;
    threeScene.add(grid);

    // Animation Loop
    function animate() {
        requestAnimationFrame(animate);
        threeControls.update();
        threeRenderer.render(threeScene, threeCamera);
    }
    animate();

    // Resize Handler
    window.addEventListener('resize', () => {
        if (!container.clientWidth || !container.clientHeight) return;
        threeCamera.aspect = container.clientWidth / container.clientHeight;
        threeCamera.updateProjectionMatrix();
        threeRenderer.setSize(container.clientWidth, container.clientHeight);
    });

    // Raycaster for ANSYS point inspection on click
    const raycaster = new THREE.Raycaster();
    raycaster.params.Points.threshold = 0.003;
    const mouse = new THREE.Vector2();

    threeRenderer.domElement.addEventListener('pointerdown', (e) => {
        if (!currentPointCloud || !pointCloudData) return;

        const rect = threeRenderer.domElement.getBoundingClientRect();
        mouse.x = ((e.clientX - rect.left) / rect.width) * 2 - 1;
        mouse.y = -((e.clientY - rect.top) / rect.height) * 2 + 1;

        raycaster.setFromCamera(mouse, threeCamera);
        const intersects = raycaster.intersectObject(currentPointCloud);

        if (intersects.length > 0) {
            const idx = intersects[0].index;
            const nid = pointCloudData.node_ids[idx];
            const px = pointCloudData.positions[idx * 3].toFixed(4);
            const py = pointCloudData.positions[idx * 3 + 1].toFixed(4);
            const pz = pointCloudData.positions[idx * 3 + 2].toFixed(4);
            const temp = pointCloudData.temperatures[idx].toFixed(2);

            document.getElementById('insp-node-id').textContent = nid;
            document.getElementById('insp-node-pos').textContent = `(${px}, ${py}, ${pz}) m`;
            document.getElementById('insp-node-temp').textContent = `${temp} °C`;
            document.getElementById('geo-inspection-panel').style.display = 'block';
        }
    });

    // Control buttons
    document.getElementById('btn-load-stl').addEventListener('click', loadSyntheticStl);
    document.getElementById('btn-load-ansys').addEventListener('click', loadSyntheticAnsys);
    document.getElementById('btn-view-reset').addEventListener('click', fitViewToModel);
    document.getElementById('btn-toggle-wireframe').addEventListener('click', toggleWireframe);

    // File upload
    const fileInput = document.getElementById('file-input');
    fileInput.addEventListener('change', (e) => {
        if (e.target.files.length > 0) {
            handleCustomFileUpload(e.target.files[0]);
        }
    });

    // Automatically load synthetic STL on startup
    loadSyntheticStl();
}

function clearCurrentGeometry() {
    if (currentMesh) {
        threeScene.remove(currentMesh);
        currentMesh.geometry.dispose();
        if (currentMesh.material) currentMesh.material.dispose();
        currentMesh = null;
    }
    if (currentPointCloud) {
        threeScene.remove(currentPointCloud);
        currentPointCloud.geometry.dispose();
        if (currentPointCloud.material) currentPointCloud.material.dispose();
        currentPointCloud = null;
    }
    pointCloudData = null;
    document.getElementById('colormap-legend').style.display = 'none';
    document.getElementById('geo-temp-range-row').style.display = 'none';
    document.getElementById('geo-inspection-panel').style.display = 'none';
    document.getElementById('geo-error-box').style.display = 'none';
}

function loadSyntheticStl() {
    clearCurrentGeometry();
    fetch('/api/geometry/load_synthetic_stl')
        .then(r => r.json())
        .then(data => {
            if (!data.success) {
                showGeoError(data.error);
                return;
            }
            renderStlModel(data);
        })
        .catch(err => showGeoError(err.message));
}

function loadSyntheticAnsys() {
    clearCurrentGeometry();
    fetch('/api/geometry/load_synthetic_ansys')
        .then(r => r.json())
        .then(data => {
            if (!data.success) {
                showGeoError(data.error);
                return;
            }
            renderAnsysPointCloud(data);
        })
        .catch(err => showGeoError(err.message));
}

function handleCustomFileUpload(file) {
    const reader = new FileReader();
    reader.onload = (e) => {
        const content = e.target.result;
        clearCurrentGeometry();

        if (file.name.toLowerCase().endsWith('.stl')) {
            fetch('/api/geometry/parse_stl_text', {
                method: 'POST',
                headers: { 'Content-Type': 'text/plain' },
                body: content
            })
            .then(r => r.json())
            .then(data => {
                if (!data.success) {
                    showGeoError(data.error);
                    return;
                }
                renderStlModel(data);
            })
            .catch(err => showGeoError(err.message));
        } else {
            // Assume CSV
            fetch('/api/geometry/parse_ansys_text', {
                method: 'POST',
                headers: { 'Content-Type': 'text/plain' },
                body: content
            })
            .then(r => r.json())
            .then(data => {
                if (!data.success) {
                    showGeoError(data.error);
                    return;
                }
                renderAnsysPointCloud(data);
            })
            .catch(err => showGeoError(err.message));
        }
    };
    reader.readAsText(file);
}

function renderStlModel(data) {
    const geom = new THREE.BufferGeometry();
    const positions = new Float32Array(data.positions);
    const normals = new Float32Array(data.normals);

    geom.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    geom.setAttribute('normal', new THREE.BufferAttribute(normals, 3));

    const mat = new THREE.MeshStandardMaterial({
        color: 0x94a3b8,
        metalness: 0.3,
        roughness: 0.5,
        wireframe: wireframeActive,
        side: THREE.DoubleSide
    });

    currentMesh = new THREE.Mesh(geom, mat);
    threeScene.add(currentMesh);

    // Update metadata card
    document.getElementById('geo-meta-type').textContent = `ASCII STL (${data.solid_name || 'solid'})`;
    document.getElementById('geo-meta-count').textContent = `${data.triangle_count} Triangles (${data.vertex_count} Vertices)`;
    document.getElementById('geo-meta-bbox-x').textContent = `[${data.bbox_min[0].toFixed(3)}, ${data.bbox_max[0].toFixed(3)}] m`;
    document.getElementById('geo-meta-bbox-y').textContent = `[${data.bbox_min[1].toFixed(3)}, ${data.bbox_max[1].toFixed(3)}] m`;
    document.getElementById('geo-meta-bbox-z').textContent = `[${data.bbox_min[2].toFixed(3)}, ${data.bbox_max[2].toFixed(3)}] m`;

    fitViewToModel();
}

function renderAnsysPointCloud(data) {
    pointCloudData = data;
    const geom = new THREE.BufferGeometry();
    const positions = new Float32Array(data.positions);
    geom.setAttribute('position', new THREE.BufferAttribute(positions, 3));

    // Colormap colors
    const colors = new Float32Array(data.positions.length);
    const minT = data.min_temperature;
    const maxT = data.max_temperature;
    const range = (maxT - minT) > 0 ? (maxT - minT) : 1.0;

    for (let i = 0; i < data.temperatures.length; ++i) {
        const val = (data.temperatures[i] - minT) / range;
        const col = getTemperatureColor(val);
        colors[i * 3] = col.r;
        colors[i * 3 + 1] = col.g;
        colors[i * 3 + 2] = col.b;
    }

    geom.setAttribute('color', new THREE.BufferAttribute(colors, 3));

    const mat = new THREE.PointsMaterial({
        size: 0.005,
        vertexColors: true
    });

    currentPointCloud = new THREE.Points(geom, mat);
    threeScene.add(currentPointCloud);

    // Update metadata & legend
    document.getElementById('geo-meta-type').textContent = 'ANSYS Thermal Point Cloud';
    document.getElementById('geo-meta-count').textContent = `${data.node_count} Nodal Points`;
    document.getElementById('geo-meta-bbox-x').textContent = `[${data.bbox_min[0].toFixed(3)}, ${data.bbox_max[0].toFixed(3)}] m`;
    document.getElementById('geo-meta-bbox-y').textContent = `[${data.bbox_min[1].toFixed(3)}, ${data.bbox_max[1].toFixed(3)}] m`;
    document.getElementById('geo-meta-bbox-z').textContent = `[${data.bbox_min[2].toFixed(3)}, ${data.bbox_max[2].toFixed(3)}] m`;
    
    document.getElementById('geo-temp-range-row').style.display = 'flex';
    document.getElementById('geo-meta-temp').textContent = `${minT.toFixed(1)} °C to ${maxT.toFixed(1)} °C`;

    // Legend
    document.getElementById('legend-min').textContent = `${minT.toFixed(1)} °C`;
    document.getElementById('legend-mid').textContent = `${((minT + maxT) * 0.5).toFixed(1)} °C`;
    document.getElementById('legend-max').textContent = `${maxT.toFixed(1)} °C`;
    document.getElementById('colormap-legend').style.display = 'block';

    fitViewToModel();
}

function getTemperatureColor(t) {
    // Jet/Rainbow colormap: Blue -> Cyan -> Green -> Yellow -> Red
    let r = 0, g = 0, b = 0;
    if (t <= 0.25) {
        const f = t / 0.25;
        b = 1.0;
        g = f;
    } else if (t <= 0.5) {
        const f = (t - 0.25) / 0.25;
        g = 1.0;
        b = 1.0 - f;
    } else if (t <= 0.75) {
        const f = (t - 0.5) / 0.25;
        g = 1.0;
        r = f;
    } else {
        const f = (t - 0.75) / 0.25;
        r = 1.0;
        g = Math.max(0, 1.0 - f);
    }
    return { r, g, b };
}

function fitViewToModel() {
    let bbox = new THREE.Box3();
    if (currentMesh) {
        bbox.setFromObject(currentMesh);
    } else if (currentPointCloud) {
        bbox.setFromObject(currentPointCloud);
    } else {
        return;
    }

    const center = new THREE.Vector3();
    bbox.getCenter(center);
    const size = new THREE.Vector3();
    bbox.getSize(size);
    const maxDim = Math.max(size.x, size.y, size.z, 0.05);

    threeControls.target.copy(center);
    threeCamera.position.set(center.x + maxDim * 1.5, center.y + maxDim * 1.2, center.z + maxDim * 2.0);
    threeCamera.lookAt(center);
    threeControls.update();
}

function toggleWireframe() {
    wireframeActive = !wireframeActive;
    if (currentMesh && currentMesh.material) {
        currentMesh.material.wireframe = wireframeActive;
    }
    document.getElementById('btn-toggle-wireframe').textContent = `Wireframe: ${wireframeActive ? 'ON' : 'OFF'}`;
}

function showGeoError(msg) {
    const el = document.getElementById('geo-error-box');
    el.textContent = `Error: ${msg}`;
    el.style.display = 'block';
}

// -------------------------------------------------------------
// 3. Thermal Analysis & FEA
// -------------------------------------------------------------
let latestFeaCsv = "";

function initThermalFea() {
    document.getElementById('btn-fea-solve').addEventListener('click', runSingleFea);
    document.getElementById('btn-fea-convergence').addEventListener('click', runConvergenceStudy);
    document.getElementById('btn-fea-download-csv').addEventListener('click', () => {
        if (latestFeaCsv) downloadCsvFile(latestFeaCsv, "fea_solution.csv");
    });

    // Auto-run convergence study on start so user sees charts immediately
    runConvergenceStudy();
}

function getFeaInputParams() {
    const L = parseFloat(document.getElementById('fea-length').value);
    const A = parseFloat(document.getElementById('fea-area').value);
    const k = parseFloat(document.getElementById('fea-k').value);
    const q = parseFloat(document.getElementById('fea-q').value);
    const T_left = parseFloat(document.getElementById('fea-tleft').value);
    const T_right = parseFloat(document.getElementById('fea-tright').value);
    const N = parseInt(document.getElementById('fea-elements').value, 10);

    if (isNaN(L) || L <= 0) throw new Error("Length L must be positive (L > 0)");
    if (isNaN(A) || A <= 0) throw new Error("Area A must be positive (A > 0)");
    if (isNaN(k) || k <= 0) throw new Error("Conductivity k must be positive (k > 0)");
    if (isNaN(N) || N < 1) throw new Error("Elements N must be >= 1");

    return { L, A, k, q, T_left, T_right, N };
}

function runSingleFea() {
    const errBox = document.getElementById('thermal-error-box');
    errBox.style.display = 'none';

    try {
        const p = getFeaInputParams();
        fetch('/api/fea/solve', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(p)
        })
        .then(r => r.json())
        .then(data => {
            if (!data.success) {
                errBox.textContent = `FEA Error: ${data.error}`;
                errBox.style.display = 'block';
                return;
            }
            latestFeaCsv = data.csv_data;
            document.getElementById('btn-fea-download-csv').disabled = false;

            renderFeaProfileChart([{
                N: p.N,
                points: data.nodes
            }]);
        })
        .catch(err => {
            errBox.textContent = `Error: ${err.message}`;
            errBox.style.display = 'block';
        });
    } catch (e) {
        errBox.textContent = `Input Validation: ${e.message}`;
        errBox.style.display = 'block';
    }
}

function runConvergenceStudy() {
    const errBox = document.getElementById('thermal-error-box');
    errBox.style.display = 'none';

    try {
        const p = getFeaInputParams();
        fetch('/api/fea/convergence', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                ...p,
                mesh_sizes: [10, 20, 40, 80]
            })
        })
        .then(r => r.json())
        .then(data => {
            if (!data.success) {
                errBox.textContent = `Convergence Error: ${data.error}`;
                errBox.style.display = 'block';
                return;
            }
            latestFeaCsv = data.csv_data;
            document.getElementById('btn-fea-download-csv').disabled = false;

            // Render table
            const tbody = document.querySelector('#convergence-table tbody');
            tbody.innerHTML = '';
            data.rows.forEach(r => {
                const tr = document.createElement('tr');
                const orderText = r.observed_order > 0 ? r.observed_order.toFixed(3) : 'Base (N/A)';
                tr.innerHTML = `
                    <td><strong>${r.N}</strong></td>
                    <td>${r.h.toFixed(4)}</td>
                    <td>${r.max_node_error.toExponential(4)}</td>
                    <td>${r.max_midpoint_error.toFixed(6)}</td>
                    <td><span class="badge badge-success">${orderText}</span></td>
                `;
                tbody.appendChild(tr);
            });

            // Render Chart
            renderFeaProfileChart(data.curves);
        })
        .catch(err => {
            errBox.textContent = `Error: ${err.message}`;
            errBox.style.display = 'block';
        });
    } catch (e) {
        errBox.textContent = `Input Validation: ${e.message}`;
        errBox.style.display = 'block';
    }
}

function renderFeaProfileChart(curves) {
    const container = document.getElementById('fea-profile-chart');
    if (!container) return;

    const width = container.clientWidth || 600;
    const height = 240;
    const pad = { top: 20, right: 120, bottom: 40, left: 60 };

    // Find x and y bounds
    let minX = 0, maxX = 1.0;
    let minY = Infinity, maxY = -Infinity;

    curves.forEach(c => {
        c.points.forEach(pt => {
            if (pt.T_fea < minY) minY = pt.T_fea;
            if (pt.T_fea > maxY) maxY = pt.T_fea;
            if (pt.T_exact < minY) minY = pt.T_exact;
            if (pt.T_exact > maxY) maxY = pt.T_exact;
        });
    });

    if (minY === Infinity) { minY = 20; maxY = 185; }
    minY = Math.floor(minY / 10) * 10;
    maxY = Math.ceil(maxY / 10) * 10;

    const mapX = (x) => pad.left + (x - minX) / (maxX - minX) * (width - pad.left - pad.right);
    const mapY = (y) => height - pad.bottom - (y - minY) / (maxY - minY) * (height - pad.top - pad.bottom);

    let svg = `<svg width="${width}" height="${height}" xmlns="http://www.w3.org/2000/svg" style="background:#171f2d; font-family:var(--font-mono); font-size:10px;">`;

    // Gridlines & Axis labels
    const ySteps = 5;
    for (let i = 0; i <= ySteps; ++i) {
        const val = minY + (maxY - minY) * (i / ySteps);
        const y = mapY(val);
        svg += `<line x1="${pad.left}" y1="${y}" x2="${width - pad.right}" y2="${y}" stroke="#2b394f" stroke-dasharray="2,2"/>`;
        svg += `<text x="${pad.left - 8}" y="${y + 3}" fill="#94a3b8" text-anchor="end">${val.toFixed(0)}°C</text>`;
    }

    const xSteps = 5;
    for (let i = 0; i <= xSteps; ++i) {
        const val = minX + (maxX - minX) * (i / xSteps);
        const x = mapX(val);
        svg += `<line x1="${x}" y1="${pad.top}" x2="${x}" y2="${height - pad.bottom}" stroke="#2b394f" stroke-dasharray="2,2"/>`;
        svg += `<text x="${x}" y="${height - pad.bottom + 16}" fill="#94a3b8" text-anchor="middle">${val.toFixed(2)} m</text>`;
    }

    // Exact curve (drawn with dashed white line)
    if (curves.length > 0 && curves[0].points.length > 0) {
        let exactPath = "";
        curves[0].points.forEach((pt, idx) => {
            const px = mapX(pt.x);
            const py = mapY(pt.T_exact);
            exactPath += (idx === 0 ? `M ${px} ${py}` : ` L ${px} ${py}`);
        });
        svg += `<path d="${exactPath}" fill="none" stroke="#ffffff" stroke-width="2" stroke-dasharray="4,4"/>`;
    }

    // Curves for each N
    const colors = ['#f59e0b', '#38bdf8', '#2dd4bf', '#a855f7'];
    curves.forEach((c, cIdx) => {
        const color = colors[cIdx % colors.length];
        let pth = "";
        c.points.forEach((pt, idx) => {
            const px = mapX(pt.x);
            const py = mapY(pt.T_fea);
            pth += (idx === 0 ? `M ${px} ${py}` : ` L ${px} ${py}`);
        });
        svg += `<path d="${pth}" fill="none" stroke="${color}" stroke-width="2"/>`;

        // Draw points for lower N
        if (c.N <= 20) {
            c.points.forEach(pt => {
                svg += `<circle cx="${mapX(pt.x)}" cy="${mapY(pt.T_fea)}" r="3" fill="${color}"/>`;
            });
        }
    });

    // Legend
    let legY = pad.top + 10;
    svg += `<g transform="translate(${width - pad.right + 10}, 0)">`;
    svg += `<line x1="0" y1="${legY}" x2="20" y2="${legY}" stroke="#ffffff" stroke-width="2" stroke-dasharray="4,4"/>`;
    svg += `<text x="25" y="${legY + 3}" fill="#f8fafc">Exact Solution</text>`;

    curves.forEach((c, idx) => {
        legY += 20;
        const col = colors[idx % colors.length];
        svg += `<line x1="0" y1="${legY}" x2="20" y2="${legY}" stroke="${col}" stroke-width="2"/>`;
        svg += `<text x="25" y="${legY + 3}" fill="${col}">FEA N=${c.N}</text>`;
    });
    svg += `</g>`;

    // Axis titles
    svg += `<text x="${pad.left + (width - pad.left - pad.right) / 2}" y="${height - 6}" fill="#cbd5e1" text-anchor="middle" font-weight="bold">Rod Position x (meters)</text>`;
    svg += `<text transform="rotate(-90)" x="${-height / 2}" y="16" fill="#cbd5e1" text-anchor="middle" font-weight="bold">Temperature T (°C)</text>`;

    svg += `</svg>`;
    container.innerHTML = svg;
}

// -------------------------------------------------------------
// 4. Backpropagation
// -------------------------------------------------------------
let latestBackpropCsv = "";

function initBackpropagation() {
    document.getElementById('btn-backprop-run').addEventListener('click', runBackprop);
    document.getElementById('btn-backprop-download-csv').addEventListener('click', () => {
        if (latestBackpropCsv) downloadCsvFile(latestBackpropCsv, "backprop_history.csv");
    });

    // Run automatically on load
    runBackprop();
}

function runBackprop() {
    fetch('/api/backprop/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ x: 1.0, y: 2.0, w: 0.5, b: 0.0, v: 1.0, c: 0.0, lr: 0.1, steps: 100 })
    })
    .then(r => r.json())
    .then(data => {
        if (!data.success) return;
        latestBackpropCsv = data.csv_data;
        document.getElementById('btn-backprop-download-csv').disabled = false;

        // Render gradient check table
        const tbody = document.querySelector('#gradcheck-table tbody');
        tbody.innerHTML = '';
        const gc = data.gradient_check;
        const params = [
            { name: 'Output Bias (c)', val: gc.c },
            { name: 'Output Weight (v)', val: gc.v },
            { name: 'Hidden Weight (w)', val: gc.w },
            { name: 'Hidden Bias (b)', val: gc.b }
        ];

        params.forEach(p => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td><strong>${p.name}</strong></td>
                <td>${p.val.analytic.toFixed(8)}</td>
                <td>${p.val.finite_diff.toFixed(8)}</td>
                <td>${p.val.abs_error.toExponential(4)}</td>
                <td><span class="badge ${p.val.passed ? 'badge-success' : 'badge-error'}">${p.val.passed ? 'PASSED (<= 1e-5)' : 'FAILED'}</span></td>
            `;
            tbody.appendChild(tr);
        });

        // Render loss chart
        renderLossChart(data.history);
    });
}

function renderLossChart(history) {
    const container = document.getElementById('backprop-loss-chart');
    if (!container || !history || history.length === 0) return;

    const width = container.clientWidth || 600;
    const height = 240;
    const pad = { top: 20, right: 30, bottom: 40, left: 60 };

    const maxLoss = history[0].loss * 1.05;
    const minLoss = 0.0;
    const maxStep = history[history.length - 1].step;

    const mapX = (s) => pad.left + (s / maxStep) * (width - pad.left - pad.right);
    const mapY = (l) => height - pad.bottom - (l / maxLoss) * (height - pad.top - pad.bottom);

    let svg = `<svg width="${width}" height="${height}" xmlns="http://www.w3.org/2000/svg" style="background:#171f2d; font-family:var(--font-mono); font-size:10px;">`;

    // Y Gridlines
    for (let i = 0; i <= 4; ++i) {
        const val = minLoss + (maxLoss - minLoss) * (i / 4);
        const y = mapY(val);
        svg += `<line x1="${pad.left}" y1="${y}" x2="${width - pad.right}" y2="${y}" stroke="#2b394f" stroke-dasharray="2,2"/>`;
        svg += `<text x="${pad.left - 8}" y="${y + 3}" fill="#94a3b8" text-anchor="end">${val.toFixed(3)}</text>`;
    }

    // X Gridlines
    for (let s = 0; s <= 100; s += 20) {
        const x = mapX(s);
        svg += `<line x1="${x}" y1="${pad.top}" x2="${x}" y2="${height - pad.bottom}" stroke="#2b394f" stroke-dasharray="2,2"/>`;
        svg += `<text x="${x}" y="${height - pad.bottom + 16}" fill="#94a3b8" text-anchor="middle">Step ${s}</text>`;
    }

    // Loss trajectory
    let pth = "";
    history.forEach((h, idx) => {
        const x = mapX(h.step);
        const y = mapY(h.loss);
        pth += (idx === 0 ? `M ${x} ${y}` : ` L ${x} ${y}`);
    });

    svg += `<path d="${pth}" fill="none" stroke="#22c55e" stroke-width="2.5"/>`;

    // Axis titles
    svg += `<text x="${pad.left + (width - pad.left - pad.right) / 2}" y="${height - 6}" fill="#cbd5e1" text-anchor="middle" font-weight="bold">Training Step Iterations (lr = 0.1)</text>`;
    svg += `<text transform="rotate(-90)" x="${-height / 2}" y="18" fill="#cbd5e1" text-anchor="middle" font-weight="bold">Loss J = 0.5(y_hat - 2)^2</text>`;

    svg += `</svg>`;
    container.innerHTML = svg;
}

// -------------------------------------------------------------
// 5. Performance & Multicore Benchmark
// -------------------------------------------------------------
let benchPollTimer = null;
let currentBenchJobId = null;
let latestBenchCsv = "";

function initPerformanceBenchmark() {
    document.getElementById('btn-bench-start').addEventListener('click', startBenchmark);
    document.getElementById('btn-bench-cancel').addEventListener('click', cancelBenchmark);
    document.getElementById('btn-bench-download-csv').addEventListener('click', () => {
        if (latestBenchCsv) downloadCsvFile(latestBenchCsv, "multicore_benchmark.csv");
    });

    // Populate initial system info
    fetch('/api/health')
        .then(() => {
            // Run a quick 1-case batch or initial fetch
            fetch('/api/multicore/start', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ case_count: 50, mesh_size: 20, seed: 42, workers: [1] })
            })
            .then(r => r.json())
            .then(data => {
                if (data.job_id) {
                    setTimeout(() => {
                        fetch(`/api/multicore/poll?job_id=${data.job_id}`)
                            .then(r => r.json())
                            .then(j => {
                                if (j.system_info) updateSystemInfoDisplay(j.system_info);
                            });
                    }, 400);
                }
            });
        });
}

function updateSystemInfoDisplay(sys) {
    document.getElementById('sys-cores').textContent = `${sys.logical_cpus} Logical Cores`;
    document.getElementById('sys-compiler').textContent = `${sys.compiler_name} ${sys.compiler_version}`;
    document.getElementById('sys-build').textContent = sys.build_config;
    document.getElementById('sys-opt').textContent = sys.optimization_level;
    document.getElementById('sys-speedup-ok').textContent = sys.speedup_assessable ? "YES" : "NO (1 CPU Core)";
}

function startBenchmark() {
    const errBox = document.getElementById('bench-error-box');
    errBox.style.display = 'none';

    const caseCount = parseInt(document.getElementById('bench-cases').value, 10);
    const meshSize = parseInt(document.getElementById('bench-mesh').value, 10);
    const seed = parseInt(document.getElementById('bench-seed').value, 10);

    const btnStart = document.getElementById('btn-bench-start');
    const btnCancel = document.getElementById('btn-bench-cancel');
    const progWrap = document.getElementById('bench-progress-wrapper');
    const progFill = document.getElementById('bench-progress-fill');
    const progText = document.getElementById('bench-progress-text');
    const statusBadge = document.getElementById('bench-status-badge');

    btnStart.disabled = true;
    btnCancel.disabled = false;
    progWrap.style.display = 'block';
    progFill.style.width = '10%';
    progText.textContent = 'Spawning bounded worker pool...';
    statusBadge.textContent = 'RUNNING';
    statusBadge.className = 'badge';

    fetch('/api/multicore/start', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            case_count: caseCount,
            mesh_size: meshSize,
            seed: seed,
            workers: [1, 2, 4]
        })
    })
    .then(r => r.json())
    .then(data => {
        if (!data.success) {
            errBox.textContent = `Benchmark Error: ${data.error}`;
            errBox.style.display = 'block';
            resetBenchUi();
            return;
        }
        currentBenchJobId = data.job_id;
        pollBenchmark(currentBenchJobId);
    })
    .catch(err => {
        errBox.textContent = `Error: ${err.message}`;
        errBox.style.display = 'block';
        resetBenchUi();
    });
}

function pollBenchmark(jobId) {
    if (benchPollTimer) clearInterval(benchPollTimer);

    benchPollTimer = setInterval(() => {
        fetch(`/api/multicore/poll?job_id=${jobId}`)
            .then(r => r.json())
            .then(data => {
                const progFill = document.getElementById('bench-progress-fill');
                const progText = document.getElementById('bench-progress-text');
                const statusBadge = document.getElementById('bench-status-badge');

                progFill.style.width = `${data.progress_pct}%`;
                progText.textContent = `Computing: ${data.progress_pct}%`;

                if (data.status === 'completed') {
                    clearInterval(benchPollTimer);
                    statusBadge.textContent = 'COMPLETED';
                    statusBadge.className = 'badge badge-success';
                    latestBenchCsv = data.csv_data;
                    document.getElementById('btn-bench-download-csv').disabled = false;
                    resetBenchUi();

                    if (data.system_info) updateSystemInfoDisplay(data.system_info);
                    renderBenchmarkResults(data);
                } else if (data.status === 'cancelled') {
                    clearInterval(benchPollTimer);
                    statusBadge.textContent = 'CANCELLED';
                    statusBadge.className = 'badge badge-error';
                    resetBenchUi();
                } else if (data.status === 'failed') {
                    clearInterval(benchPollTimer);
                    statusBadge.textContent = 'FAILED';
                    statusBadge.className = 'badge badge-error';
                    document.getElementById('bench-error-box').textContent = data.error_message || 'Benchmark execution failed';
                    document.getElementById('bench-error-box').style.display = 'block';
                    resetBenchUi();
                }
            })
            .catch(() => {
                clearInterval(benchPollTimer);
                resetBenchUi();
            });
    }, 300);
}

function cancelBenchmark() {
    if (currentBenchJobId) {
        fetch(`/api/multicore/cancel?job_id=${currentBenchJobId}`, { method: 'POST' });
    }
}

function resetBenchUi() {
    document.getElementById('btn-bench-start').disabled = false;
    document.getElementById('btn-bench-cancel').disabled = true;
}

function renderBenchmarkResults(data) {
    // Verification banner
    const vBanner = document.getElementById('bench-verification-banner');
    const vText = document.getElementById('bench-verification-text');
    vBanner.style.display = 'flex';
    vText.textContent = `Serial vs Parallel Verification: Max Abs Diff = ${data.verification.max_abs_diff.toExponential(4)} (PASSED <= 1e-8)`;

    // Results table
    const tbody = document.querySelector('#bench-results-table tbody');
    tbody.innerHTML = '';

    data.runs.forEach(r => {
        const tr = document.createElement('tr');
        const repStr = r.repetition_times_ms.map(t => t.toFixed(2)).join(', ');
        tr.innerHTML = `
            <td><strong>${r.workers} Worker${r.workers > 1 ? 's' : ''}</strong></td>
            <td><small>${repStr}</small></td>
            <td><strong>${r.median_time_ms.toFixed(2)} ms</strong></td>
            <td><span class="badge ${r.speedup >= 1.0 ? 'badge-success' : ''}">${r.speedup.toFixed(2)}x</span></td>
            <td>${(r.efficiency * 100).toFixed(1)}%</td>
        `;
        tbody.appendChild(tr);
    });

    renderSpeedupChart(data.runs);
}

function renderSpeedupChart(runs) {
    const container = document.getElementById('bench-speedup-chart');
    if (!container || !runs || runs.length === 0) return;

    const width = container.clientWidth || 600;
    const height = 240;
    const pad = { top: 20, right: 120, bottom: 40, left: 60 };

    const maxWorkers = runs[runs.length - 1].workers;
    const maxSpeedup = Math.max(maxWorkers, 4.0);

    const mapX = (w) => pad.left + ((w - 1) / (maxWorkers - 1 || 1)) * (width - pad.left - pad.right);
    const mapY = (s) => height - pad.bottom - (s / maxSpeedup) * (height - pad.top - pad.bottom);

    let svg = `<svg width="${width}" height="${height}" xmlns="http://www.w3.org/2000/svg" style="background:#171f2d; font-family:var(--font-mono); font-size:10px;">`;

    // Gridlines
    for (let s = 1; s <= maxSpeedup; ++s) {
        const y = mapY(s);
        svg += `<line x1="${pad.left}" y1="${y}" x2="${width - pad.right}" y2="${y}" stroke="#2b394f" stroke-dasharray="2,2"/>`;
        svg += `<text x="${pad.left - 8}" y="${y + 3}" fill="#94a3b8" text-anchor="end">${s.toFixed(1)}x</text>`;
    }

    // Ideal linear speedup line
    const idealX1 = mapX(1), idealY1 = mapY(1);
    const idealX2 = mapX(maxWorkers), idealY2 = mapY(maxWorkers);
    svg += `<line x1="${idealX1}" y1="${idealY1}" x2="${idealX2}" y2="${idealY2}" stroke="#ffffff" stroke-width="2" stroke-dasharray="4,4"/>`;

    // Measured speedup path
    let pth = "";
    runs.forEach((r, idx) => {
        const x = mapX(r.workers);
        const y = mapY(r.speedup);
        pth += (idx === 0 ? `M ${x} ${y}` : ` L ${x} ${y}`);
    });
    svg += `<path d="${pth}" fill="none" stroke="#38bdf8" stroke-width="2.5"/>`;

    // Measured points
    runs.forEach(r => {
        const x = mapX(r.workers);
        const y = mapY(r.speedup);
        svg += `<circle cx="${x}" cy="${y}" r="4" fill="#38bdf8"/>`;
        svg += `<text x="${x}" y="${y - 10}" fill="#38bdf8" text-anchor="middle" font-weight="bold">${r.speedup.toFixed(2)}x</text>`;
        svg += `<text x="${x}" y="${height - pad.bottom + 16}" fill="#94a3b8" text-anchor="middle">${r.workers} Core${r.workers > 1 ? 's' : ''}</text>`;
    });

    // Legend
    svg += `<g transform="translate(${width - pad.right + 10}, ${pad.top + 20})">`;
    svg += `<line x1="0" y1="0" x2="20" y2="0" stroke="#ffffff" stroke-width="2" stroke-dasharray="4,4"/>`;
    svg += `<text x="25" y="3" fill="#f8fafc">Ideal Linear</text>`;
    svg += `<line x1="0" y1="20" x2="20" y2="20" stroke="#38bdf8" stroke-width="2.5"/>`;
    svg += `<text x="25" y="23" fill="#38bdf8">Measured</text>`;
    svg += `</g>`;

    // Titles
    svg += `<text x="${pad.left + (width - pad.left - pad.right) / 2}" y="${height - 6}" fill="#cbd5e1" text-anchor="middle" font-weight="bold">Active Worker Threads (Bounded Pool)</text>`;
    svg += `<text transform="rotate(-90)" x="${-height / 2}" y="18" fill="#cbd5e1" text-anchor="middle" font-weight="bold">Speedup Factor S_p</text>`;

    svg += `</svg>`;
    container.innerHTML = svg;
}

// -------------------------------------------------------------
// 6. Hardware (Simulated) Tab
// -------------------------------------------------------------
const activeTelemetryTimers = {};

function initHardwareSim() {
    setupDeviceCard('mcu', 'mcu_stm32h7');
    setupDeviceCard('fpga', 'fpga_artix7');
    setupDeviceCard('jtag', 'jtag_ft2232h');

    // Poll initial device states
    fetch('/api/hardware/devices')
        .then(r => r.json())
        .then(data => {
            if (data.devices) {
                data.devices.forEach(d => {
                    const prefix = d.id.startsWith('mcu') ? 'mcu' : d.id.startsWith('fpga') ? 'fpga' : 'jtag';
                    updateDeviceCardStatus(prefix, d.state);
                });
            }
        });
}

function setupDeviceCard(prefix, devId) {
    const btnConnect = document.getElementById(`btn-${prefix}-connect`);
    const btnDisconnect = document.getElementById(`btn-${prefix}-disconnect`);
    const btnFail = document.getElementById(`btn-${prefix}-fail`);
    const btnRecover = document.getElementById(`btn-${prefix}-recover`);

    btnConnect.addEventListener('click', () => {
        fetch('/api/hardware/connect', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: devId })
        }).then(() => {
            updateDeviceCardStatus(prefix, 'CONNECTED');
            startDevicePolling(prefix, devId);
        });
    });

    btnDisconnect.addEventListener('click', () => {
        fetch('/api/hardware/disconnect', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: devId })
        }).then(() => {
            stopDevicePolling(prefix);
            updateDeviceCardStatus(prefix, 'DISCONNECTED');
        });
    });

    btnFail.addEventListener('click', () => {
        fetch('/api/hardware/trigger_failure', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: devId })
        }).then(() => {
            updateDeviceCardStatus(prefix, 'ERROR_TIMEOUT');
        });
    });

    btnRecover.addEventListener('click', () => {
        fetch('/api/hardware/recover', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: devId })
        }).then(() => {
            updateDeviceCardStatus(prefix, 'CONNECTED');
        });
    });
}

function updateDeviceCardStatus(prefix, state) {
    const badge = document.getElementById(`${prefix}-status-badge`);
    const btnConnect = document.getElementById(`btn-${prefix}-connect`);
    const btnDisconnect = document.getElementById(`btn-${prefix}-disconnect`);
    const btnFail = document.getElementById(`btn-${prefix}-fail`);
    const btnRecover = document.getElementById(`btn-${prefix}-recover`);
    const errMsg = document.getElementById(`${prefix}-error-msg`);

    badge.textContent = state;

    if (state === 'CONNECTED') {
        badge.className = 'badge badge-success';
        btnConnect.disabled = true;
        btnDisconnect.disabled = false;
        btnFail.disabled = false;
        btnRecover.disabled = true;
        errMsg.style.display = 'none';
    } else if (state === 'ERROR_TIMEOUT') {
        badge.className = 'badge badge-error';
        btnConnect.disabled = true;
        btnDisconnect.disabled = false;
        btnFail.disabled = true;
        btnRecover.disabled = false;
    } else {
        badge.className = 'badge';
        btnConnect.disabled = false;
        btnDisconnect.disabled = true;
        btnFail.disabled = true;
        btnRecover.disabled = true;
        errMsg.style.display = 'none';
    }
}

function startDevicePolling(prefix, devId) {
    stopDevicePolling(prefix);

    const poll = () => {
        fetch(`/api/hardware/telemetry?id=${devId}`)
            .then(r => r.json())
            .then(data => {
                updateDeviceCardStatus(prefix, data.state);

                if (data.state === 'ERROR_TIMEOUT') {
                    const errEl = document.getElementById(`${prefix}-error-msg`);
                    errEl.textContent = `Fault Injection Active: ${data.last_error_message}`;
                    errEl.style.display = 'block';
                }

                if (prefix === 'mcu' && data.numeric_metrics) {
                    if (data.numeric_metrics.core_temperature_C)
                        document.getElementById('mcu-temp').textContent = `${data.numeric_metrics.core_temperature_C.toFixed(1)} °C`;
                    if (data.numeric_metrics.vcc_core_V)
                        document.getElementById('mcu-vcc-core').textContent = `${data.numeric_metrics.vcc_core_V.toFixed(3)} V`;
                    if (data.numeric_metrics.vcc_io_3v3_V)
                        document.getElementById('mcu-vcc-io').textContent = `${data.numeric_metrics.vcc_io_3v3_V.toFixed(3)} V`;
                    if (data.numeric_metrics.clock_freq_MHz)
                        document.getElementById('mcu-clock').textContent = `${data.numeric_metrics.clock_freq_MHz.toFixed(0)} MHz`;
                    if (data.numeric_metrics.samples_processed)
                        document.getElementById('mcu-samples').textContent = data.numeric_metrics.samples_processed.toLocaleString();
                    if (data.status_metrics)
                        document.getElementById('mcu-fw').textContent = `${data.status_metrics.firmware_version || ''} (${data.status_metrics.watchdog_status || 'OK'})`;
                } else if (prefix === 'fpga' && data.numeric_metrics) {
                    if (data.numeric_metrics.die_temperature_C)
                        document.getElementById('fpga-temp').textContent = `${data.numeric_metrics.die_temperature_C.toFixed(1)} °C`;
                    if (data.numeric_metrics.vcc_int_1v0_V)
                        document.getElementById('fpga-vcc-int').textContent = `${data.numeric_metrics.vcc_int_1v0_V.toFixed(3)} V`;
                    if (data.numeric_metrics.vcc_aux_1v8_V)
                        document.getElementById('fpga-vcc-aux').textContent = `${data.numeric_metrics.vcc_aux_1v8_V.toFixed(3)} V`;
                    if (data.numeric_metrics.lut_utilization_pct)
                        document.getElementById('fpga-usage').textContent = `LUT: ${data.numeric_metrics.lut_utilization_pct}% / DSP: ${data.numeric_metrics.dsp_utilization_pct}%`;
                    if (data.status_metrics)
                        document.getElementById('fpga-accel').textContent = data.status_metrics.bitstream_id || 'ACTIVE';
                    if (data.status_metrics)
                        document.getElementById('fpga-sysmon').textContent = data.status_metrics.sysmon_alarm || 'NOMINAL';
                } else if (prefix === 'jtag' && data.numeric_metrics) {
                    if (data.numeric_metrics.tck_frequency_kHz)
                        document.getElementById('jtag-tck').textContent = `${(data.numeric_metrics.tck_frequency_kHz / 1000).toFixed(1)} MHz`;
                    if (data.numeric_metrics.sensed_vref_V)
                        document.getElementById('jtag-vref').textContent = `${data.numeric_metrics.sensed_vref_V.toFixed(2)} V`;
                    if (data.status_metrics)
                        document.getElementById('jtag-tap').textContent = data.status_metrics.tap_state || 'IDLE';
                    if (data.numeric_metrics.scanned_chain_devices)
                        document.getElementById('jtag-targets').textContent = `${data.numeric_metrics.scanned_chain_devices} TAP Devices`;
                    if (data.status_metrics) {
                        document.getElementById('jtag-dev0').textContent = data.status_metrics.device_0_idcode || 'ARM CoreSight';
                        document.getElementById('jtag-dev1').textContent = data.status_metrics.device_1_idcode || 'Artix-7 FPGA';
                    }
                }
            });
    };

    poll();
    activeTelemetryTimers[prefix] = setInterval(poll, 1000);
}

function stopDevicePolling(prefix) {
    if (activeTelemetryTimers[prefix]) {
        clearInterval(activeTelemetryTimers[prefix]);
        delete activeTelemetryTimers[prefix];
    }
}

// -------------------------------------------------------------
// 7. Utility: Download CSV file
// -------------------------------------------------------------
function downloadCsvFile(csvContent, filename) {
    const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.setAttribute('download', filename);
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
}
