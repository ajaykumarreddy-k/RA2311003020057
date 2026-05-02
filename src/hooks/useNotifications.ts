import { useState, useEffect } from 'react';
import { fetchNotifications } from '../lib/api';

export function useNotifications(params: object) {
  const [data, setData] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');

  useEffect(() => {
    setLoading(true);
    fetchNotifications(params)
      .then(setData)
      .catch(e => setError(e.message))
      .finally(() => setLoading(false));
  }, [JSON.stringify(params)]);

  return { data, loading, error };
}
