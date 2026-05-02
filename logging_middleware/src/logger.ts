/**
 * logging_middleware/src/logger.ts
 * Core LoggerClient — handles auth token lifecycle and the Log() function.
 */

import axios, { AxiosInstance } from "axios";
import { Level, LoggerConfig, LogPayload, Package, Stack, AuthResponse } from "./types";

const DEFAULT_BASE_URL = "http://20.207.122.201";
const AUTH_PATH = "/evaluation-service/auth";
const LOGS_PATH = "/evaluation-service/logs";
const TOKEN_BUFFER_S = 60; // refresh 60 s before expiry

export class LoggerClient {
  private readonly http: AxiosInstance;
  private readonly config: Required<LoggerConfig>;

  private accessToken: string | null = null;
  private tokenExpiresAt: number = 0; // Unix seconds

  constructor(config: LoggerConfig) {
    this.config = {
      baseUrl: config.baseUrl ?? DEFAULT_BASE_URL,
      email: config.email,
      name: config.name,
      rollNo: config.rollNo,
      accessCode: config.accessCode,
      clientID: config.clientID,
      clientSecret: config.clientSecret,
      consoleEcho: config.consoleEcho ?? true,
    };

    this.http = axios.create({
      baseURL: this.config.baseUrl,
      timeout: 10_000,
      headers: { "Content-Type": "application/json" },
    });
  }

  /**
   * Log(stack, level, package, message)
   * Sends a structured log entry to the evaluation service.
   */
  async Log(stack: Stack, level: Level, pkg: Package, message: string): Promise<void> {
    if (this.config.consoleEcho) {
      this.echoToConsole(level, `[${stack}][${level}][${pkg}]`, message);
    }

    const payload: LogPayload = { stack, level, package: pkg, message };

    try {
      const token = await this.getToken();
      await this.http.post(LOGS_PATH, payload, {
        headers: { Authorization: `Bearer ${token}` },
      });
    } catch (err: unknown) {
      // Swallow logging errors — never crash the consuming app
      console.warn("[logging-middleware] Failed to send log:", err instanceof Error ? err.message : err);
    }
  }

  // ── Token management ───────────────────────────────────────────────────

  private async getToken(): Promise<string> {
    const nowS = Math.floor(Date.now() / 1000);
    if (this.accessToken && nowS < this.tokenExpiresAt - TOKEN_BUFFER_S) {
      return this.accessToken;
    }
    return this.refreshToken();
  }

  private async refreshToken(): Promise<string> {
    const response = await this.http.post<AuthResponse>(AUTH_PATH, {
      email:        this.config.email.toLowerCase(),
      name:         this.config.name.toLowerCase(),
      rollNo:       this.config.rollNo.toLowerCase(),
      accessCode:   this.config.accessCode,
      clientID:     this.config.clientID,
      clientSecret: this.config.clientSecret,
    });

    const { access_token, expires_in } = response.data;
    this.accessToken = access_token;
    // expires_in is a Unix timestamp in seconds
    this.tokenExpiresAt = expires_in;
    return access_token;
  }

  // ── Console echo ───────────────────────────────────────────────────────

  private echoToConsole(level: Level, label: string, message: string): void {
    switch (level) {
      case "debug":   console.debug(label, message); break;
      case "info":    console.info(label, message);  break;
      case "warn":    console.warn(label, message);  break;
      case "error":
      case "fatal":   console.error(label, message); break;
    }
  }
}
