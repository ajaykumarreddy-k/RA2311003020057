import { useState, useEffect } from 'react';
import { fetchNotifications } from '../lib/api';
import { Log } from '../lib/logger';

export function useNotifications(params: object) {
  const [data, setData] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');

  useEffect(() => {
    Log('frontend', 'info', 'middleware', 'useNotifications hook initialized');
    setLoading(true);
    fetchNotifications(params)
      .then(res => {
        setData(res);
        Log('frontend', 'debug', 'middleware', 'Data successfully set in component state');
      })
      .catch(e => {
        setError(e.message);
        Log('frontend', 'error', 'middleware', `Error propagated to component state: ${e.message}`);
      })
      .finally(() => setLoading(false));
  }, [JSON.stringify(params)]);

  return { data, loading, error };
}
