/**
 * logging_middleware/src/types.ts
 * Type definitions for the logging middleware package.
 */

export type Stack = "frontend" | "backend";
export type Level = "debug" | "info" | "warn" | "error" | "fatal";
export type Package =
  | "api" | "component" | "hook" | "page" | "state" | "style"
  | "cache" | "controller" | "cron_job" | "db" | "domain"
  | "handler" | "repository" | "route" | "service"
  | "auth" | "config" | "middleware" | "utils";

export interface LogPayload {
  stack: Stack;
  level: Level;
  package: Package;
  message: string;
}

/** Response from POST /evaluation-service/auth */
export interface AuthResponse {
  token_type: string;
  access_token: string;
  expires_in: number; // Unix timestamp in seconds (NOT duration)
}

/** All fields needed for the auth request body */
export interface LoggerConfig {
  baseUrl?: string;
  email: string;
  name: string;
  rollNo: string;
  accessCode: string;
  clientID: string;
  clientSecret: string;
  consoleEcho?: boolean;
}
