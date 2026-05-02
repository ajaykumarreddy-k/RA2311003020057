# Notification System Design (Frontend Track)

## 1. System Architecture Overview
The Campus Notifications platform is designed as a decoupled, responsive single-page application (SPA). It strictly follows modern React paradigms, utilizing React 19 and Vite 6 for high performance. 

### Core Components:
1. **Frontend Application (`notification_app_fe`)**: A React/Vite application built with Tailwind CSS (substituting MUI for compatibility and performance).
2. **Evaluation Server (External API)**: The central source of truth for notification data (`http://20.207.122.201`).
3. **Logging Middleware (`logging_middleware`)**: An isolated, reusable TypeScript module that automatically handles authentication and pushes structural logs to the protected evaluation service.

---

## 2. Component Design & State Management

*   **`useNotifications` Hook:** Acts as the central data-fetching controller. It manages loading states, error propagation, and caches the returned data to prevent redundant network calls.
*   **Vite Proxy:** All outbound API requests from the frontend are routed through Vite's local proxy. This completely eliminates Cross-Origin Resource Sharing (CORS) blocks without exposing security tokens.
*   **Dynamic Theming:** Tailwind CSS is configured to strictly enforce the Google Material 3 design system, maintaining visual consistency across devices.

---

## 3. Data Flow

1. **Initialization:** The user visits `http://localhost:3000`. The `logging_middleware` automatically initializes, securely fetching a Bearer token via the `/evaluation-service/auth` endpoint.
2. **Data Fetching:** The `Home` or `Priority` page mounts. `useNotifications` fires a GET request to `/evaluation-service/notifications`.
3. **Proxy Intercept:** The Vite server intercepts the relative API call and proxies it to the remote evaluation server.
4. **Render:** Data is returned, sorted (by weight and read-status for Priority inbox), and rendered via the `NotificationCard` components.
5. **Logging:** Throughout this lifecycle, the `LoggerClient` continuously posts "info", "debug", and "error" telemetry securely back to the `/evaluation-service/logs` endpoint.

---

## 4. Error Handling & Resilience
*   **Graceful Degradation:** If the evaluation server goes down, the `useNotifications` hook catches the network failure and updates the UI to display a user-friendly error state instead of a blank screen.
*   **Non-Blocking Logs:** The logging middleware uses isolated `try/catch` blocks. If the telemetry server fails, the logs fail silently (with a local console warning), ensuring the core application never crashes due to a telemetry failure.
