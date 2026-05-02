import { useState } from 'react';
import { Container, Select, MenuItem, Typography, Box, CircularProgress } from '@mui/material';
import { useNotifications } from '../hooks/useNotifications';
import NotificationCard from '../components/NotificationCard';

const WEIGHTS: Record<string, number> = { Placement: 3, Result: 2, Event: 1 };

function score(n: any) {
  return (WEIGHTS[n.type] || 1) * (n.is_read ? 1 : 2);
}

export default function Priority() {
  const [topN, setTopN] = useState(10);
  const { data, loading, error } = useNotifications({ limit: 100 });

  const sorted = [...(data || [])].sort((a, b) => score(b) - score(a)).slice(0, topN);

  return (
    <Container maxWidth="md" sx={{ mt: 4 }}>
      <Typography variant="h4" mb={3} fontWeight="bold" fontFamily="system-ui, -apple-system, sans-serif">
        Priority Inbox
      </Typography>
      
      <Box mb={3} display="flex" alignItems="center">
        <Typography variant="body1" mr={2} color="text.secondary">Show Top:</Typography>
        <Select 
          value={topN} 
          onChange={e => setTopN(Number(e.target.value))} 
          size="small" 
          sx={{ minWidth: 120, bgcolor: 'white' }}
        >
          {[10, 15, 20].map(n => <MenuItem key={n} value={n}>Top {n}</MenuItem>)}
        </Select>
      </Box>

      {loading && (
        <Box display="flex" justifyContent="center" my={4}>
          <CircularProgress />
        </Box>
      )}
      {error && <Typography color="error" my={2}>{error}</Typography>}
      
      {!loading && !error && sorted.map((n: any) => (
        <NotificationCard key={n.id} n={n} viewed={n.is_read} />
      ))}

      {!loading && !error && sorted.length === 0 && (
        <Typography variant="body1" color="text.secondary" align="center" my={4}>
          No high-priority notifications found.
        </Typography>
      )}
    </Container>
  );
}
