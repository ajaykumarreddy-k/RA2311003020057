import { LoggerClient } from 'logging-middleware';
import type { Stack, Level, Package } from 'logging-middleware/dist/types';

export type LogStack = Stack;
export type LogLevel = Level;
export type LogPackage = Package;

// Initialize using environment variables that the user will configure
const logger = new LoggerClient({
  baseUrl: '', // Empty string forces Axios to use relative paths (hitting the Vite proxy)
  email: import.meta.env.VITE_AUTH_EMAIL || '',
  name: import.meta.env.VITE_AUTH_NAME || '',
  rollNo: import.meta.env.VITE_AUTH_ROLLNO || '',
  accessCode: import.meta.env.VITE_AUTH_ACCESS_CODE || '',
  clientID: import.meta.env.VITE_AUTH_CLIENT_ID || '',
  clientSecret: import.meta.env.VITE_AUTH_CLIENT_SECRET || '',
  consoleEcho: true,
});

export async function Log(stack: LogStack, level: LogLevel, pkg: LogPackage, message: string) {
  await logger.Log(stack, level, pkg, message);
}

