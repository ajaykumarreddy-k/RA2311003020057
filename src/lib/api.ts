import logger from './logger';

const BASE = 'http://20.207.122.201/evaluation-service/notifications';

export async function fetchNotifications(params: {
  limit?: number; page?: number; notification_type?: string;
}) {
  const url = new URL(BASE);
  Object.entries(params).forEach(([k, v]) => {
    if (v) url.searchParams.set(k, String(v));
  });
  logger.info(`GET ${url}`);
  const res = await fetch(url.toString());
  if (!res.ok) throw new Error(`API error: ${res.status}`);
  return res.json();
}
