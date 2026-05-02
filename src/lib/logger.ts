const logger = {
  info: (msg: string) => console.info(`[INFO] ${new Date().toISOString()} - ${msg}`),
  error: (msg: string) => console.error(`[ERROR] ${new Date().toISOString()} - ${msg}`),
};

export default logger;
