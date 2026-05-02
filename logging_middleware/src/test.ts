/**
 * logging_middleware/src/test.ts
 * Smoke-test: run with  CLIENT_ID=x CLIENT_SECRET=y ... npx ts-node src/test.ts
 */
import { LoggerClient } from "./logger";

const logger = new LoggerClient({
  email:        process.env.EMAIL         ?? "your@email.com",
  name:         process.env.NAME          ?? "Your Name",
  rollNo:       process.env.ROLL_NO       ?? "RA0000000000000",
  accessCode:   process.env.ACCESS_CODE   ?? "XXXXXX",
  clientID:     process.env.CLIENT_ID     ?? "YOUR_CLIENT_ID",
  clientSecret: process.env.CLIENT_SECRET ?? "YOUR_CLIENT_SECRET",
  consoleEcho:  true,
});

async function main() {
  console.log("=== Logging Middleware Smoke Test ===\n");
  await logger.Log("frontend", "debug",  "config",     "Logger initialised — smoke test starting");
  await logger.Log("frontend", "info",   "middleware",  "Smoke test: info level");
  await logger.Log("frontend", "warn",   "utils",       "Smoke test: warn level");
  await logger.Log("frontend", "error",  "api",         "Smoke test: error level");
  await logger.Log("frontend", "fatal",  "auth",        "Smoke test: fatal level");
  console.log("\n=== Done ===");
}

main().catch(console.error);
