import { Log } from './logger';

const BASE = import.meta.env.VITE_API_URL || '/evaluation-service/notifications';

export async function fetchNotifications(params: {
  limit?: number; page?: number; notification_type?: string;
}) {
  const url = new URL(BASE, window.location.origin);
  Object.entries(params).forEach(([k, v]) => {
    if (v) url.searchParams.set(k, String(v));
  });
  
  Log('frontend', 'info', 'utils', `Fetching notifications with params: ${JSON.stringify(params)}`);
  
  try {
    const res = await fetch(url.toString());
    if (!res.ok) {
      Log('frontend', 'error', 'utils', `API returned status: ${res.status}`);
      throw new Error(`API error: ${res.status}`);
    }
    const data = await res.json();
    Log('frontend', 'debug', 'utils', `Successfully fetched ${data?.length || 0} notifications`);
    return data;
  } catch (err: any) {
    Log('frontend', 'error', 'utils', `Network or parsing error: ${err.message}`);
    throw err;
  }
}
