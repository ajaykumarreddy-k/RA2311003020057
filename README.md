<div align="center">
  <img src="./Assets /Priority.gif" alt="Logo" width="80" height="80">
  <h1 align="center">Campus Notifications Platform</h1>
  <p align="center">
    A centralized, high-performance notification portal built for modern university campuses.
    <br />
    <a href="#architecture--data-flow"><strong>Explore the Architecture »</strong></a>
    <br />
    <br />
    <a href="#performance-metrics">View Metrics</a>
    ·
    <a href="#overcoming-the-401-auth-error">Read Auth Solution</a>
  </p>
</div>

<!-- Badges -->
<div align="center">
  <img src="https://img.shields.io/badge/React-19.0.1-61DAFB?style=for-the-badge&logo=react&logoColor=black" alt="React">
  <img src="https://img.shields.io/badge/TypeScript-5.8.2-3178C6?style=for-the-badge&logo=typescript&logoColor=white" alt="TypeScript">
  <img src="https://img.shields.io/badge/Vite-6.2.3-646CFF?style=for-the-badge&logo=vite&logoColor=white" alt="Vite">
  <img src="https://img.shields.io/badge/Tailwind_CSS-4.1-38B2AC?style=for-the-badge&logo=tailwind-css&logoColor=white" alt="Tailwind CSS">
</div>

---

## 🎬 Platform Preview

<div align="center">
  <h3>Live Walkthrough</h3>
  <video src="https://raw.githubusercontent.com/ajaykumarreddy-k/RA2311003020057/main/Assets%20/Screencast%20From%202026-05-02%2012-15-23.webm" width="100%" controls="controls" autoplay="autoplay" loop="loop" muted="muted">
    <a href="https://raw.githubusercontent.com/ajaykumarreddy-k/RA2311003020057/main/Assets%20/Screencast%20From%202026-05-02%2012-15-23.webm">View Demo Video</a>
  </video>
</div>

### Screenshots
<div align="center">
  <img src="./Assets /screencapture-localhost-3000-2026-05-02-12_16_54.png" alt="All Notifications View" width="49%" />
  &nbsp;
  <img src="./Assets /screencapture-localhost-3000-priority-2026-05-02-12_17_08.png" alt="Priority Notifications View" width="49%" />
</div>

---

## ⚡ Performance Metrics

The application is hyper-optimized for performance, achieving near-instantaneous load times and perfect layout stability. Based on local profiling:

*   🟢 **Largest Contentful Paint (LCP):** `0.13 s` (Excellent)
*   🟢 **Cumulative Layout Shift (CLS):** `0` (Perfect visual stability)
*   🟢 **Interaction to Next Paint (INP):** `0 ms` (Instant responsiveness)

---

## 🏗 Architecture & Data Flow

### The Flow
1. **User Interaction:** The user interacts with the React frontend (e.g., changing the filter to "Placement").
2. **API Request:** The `useNotifications` hook triggers a fetch call via `src/lib/api.ts`.
3. **Vite Proxy:** The request is sent to a relative path (`/evaluation-service/notifications`) and intercepted by the Vite dev server proxy to bypass CORS restrictions.
4. **Test Server:** The request hits `http://20.207.122.201` and returns the paginated/filtered data.
5. **Logging:** Concurrently, the `logging-middleware` authenticates and posts a lifecycle log to the protected `/evaluation-service/logs` endpoint.

### Sample API Fetches

**Fetch All Notifications (Default):**
```bash
curl -X GET "http://20.207.122.201/evaluation-service/notifications" \
  -H "Content-Type: application/json"
```

**Filter by Type & Paginate:**
```bash
curl -X GET "http://20.207.122.201/evaluation-service/notifications?notification_type=Placement&page=1&limit=10" \
  -H "Content-Type: application/json"
```

---

## 🔐 Overcoming the 401 Auth Error

During development, integrating the logging middleware resulted in a persistent **`API error: 401 (Unauthorized)`**. 

**The Cause:** 
The target API for logging (`/evaluation-service/logs`) is a strictly protected route. Our initial custom `fetch` implementation was attempting to POST logs without a valid Bearer token, causing the server to reject the requests.

**The Solution:**
1. **Middleware Integration:** We pivoted to fully utilizing the provided local `logging-middleware` package. 
2. **Auto-Authentication:** This package acts as a `LoggerClient` that automatically hits the `/evaluation-service/auth` endpoint with the user's credentials (injected securely via `.env`) to retrieve and cache an `access_token`.
3. **Proxy Passthrough:** We configured the `LoggerClient`'s `baseUrl` to an empty string, forcing the authentication and logging requests to route through our Vite proxy. This securely resolved the 401 Auth error while entirely sidestepping CORS blocks.

---

## 🚀 Getting Started

### Prerequisites
*   Node.js (v18+)
*   npm or bun

### Installation

1.  **Clone & Install Dependencies**
    ```bash
    git clone https://github.com/ajaykumarreddy-k/RA2311003020057.git
    cd RA2311003020057
    npm install
    ```
2.  **Configure Environment Variables**
    Create a `.env` file at the root:
    ```env
    VITE_API_URL=/evaluation-service/notifications
    
    # Logging Auth Credentials
    VITE_AUTH_EMAIL="participant@example.com"
    VITE_AUTH_NAME="Participant"
    VITE_AUTH_ROLLNO="Roll_Number"
    VITE_AUTH_ACCESS_CODE="Access_Code"
    VITE_AUTH_CLIENT_ID="Client_ID"
    VITE_AUTH_CLIENT_SECRET="Client_Secret"
    ```
3.  **Run the Server**
    ```bash
    npm run dev
    ```
    *Runs strictly on `http://localhost:3000`.*

---

## 🎨 Design Decisions

**Styling Framework (Tailwind vs. MUI)**
While the original project constraints suggested the use of Material UI (MUI), severe module resolution and runtime compatibility issues with the latest React 19 + Vite 6 environment caused application crashes. To ensure a **robust, error-free production build**, the UI was refactored using **Tailwind CSS**. 

We achieved the exact required **Material Design 3 aesthetic** (including custom chips, elevation shadows, Google colors, and Product Sans typography) purely through highly optimized native CSS utilities. This guarantees maximum performance (as seen in our metrics) without the bulk of external UI libraries.

---

<div align="center">
  <p>Built with ❤️ for Campus Connect. Project ID: RA2311003020057</p>
</div>
