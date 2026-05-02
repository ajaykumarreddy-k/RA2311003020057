# Logging Middleware

A reusable TypeScript package that provides structured logging to the evaluation service API.

## Features

- **Type-safe** — strict TypeScript types for `stack`, `level`, and `package`
- **Auto-auth** — automatically fetches and refreshes Bearer tokens
- **Resilient** — logging failures never crash the consuming application
- **Console echo** — mirrors every log entry to the browser/Node console

## Installation

```bash
npm install
npm run build
```

## Usage

```typescript
import { LoggerClient } from "logging-middleware";

const logger = new LoggerClient({
  clientId: "YOUR_CLIENT_ID",
  clientSecret: "YOUR_CLIENT_SECRET",
  consoleEcho: true, // optional, default true
});

// Log(stack, level, package, message)
await logger.Log("frontend", "info",  "component", "Header component mounted");
await logger.Log("frontend", "warn",  "api",       "Slow API response detected");
await logger.Log("frontend", "error", "hook",      "useNotifications: fetch failed");
```

## API Reference

### `Log(stack, level, package, message)`

| Parameter | Type     | Allowed values |
|-----------|----------|----------------|
| `stack`   | `Stack`  | `"frontend"` |
| `level`   | `Level`  | `"debug"` \| `"info"` \| `"warn"` \| `"error"` \| `"fatal"` |
| `package` | `Package`| `"api"` \| `"component"` \| `"hook"` \| `"page"` \| `"state"` \| `"style"` \| `"auth"` \| `"config"` \| `"middleware"` \| `"utils"` |
| `message` | `string` | Human-readable log description |

## Running the Smoke Test

```bash
CLIENT_ID=xxx CLIENT_SECRET=yyy npx ts-node src/test.ts
```
