import { useState } from 'react';
import { Container, Select, MenuItem, Pagination, CircularProgress, Typography, Box } from '@mui/material';
import { useNotifications } from '../hooks/useNotifications';
import NotificationCard from '../components/NotificationCard';

export default function Home() {
  const [page, setPage] = useState(1);
  const [type, setType] = useState('');
  const [viewed, setViewed] = useState<Set<string>>(new Set());
  const { data, loading, error } = useNotifications({ limit: 10, page, notification_type: type });

  const markViewed = (id: string) => setViewed(prev => new Set([...prev, id]));

  return (
    <Container maxWidth="md" sx={{ mt: 4 }}>
      <Typography variant="h4" mb={3} fontWeight="bold" fontFamily="system-ui, -apple-system, sans-serif">
        All Notifications
      </Typography>
      
      <Box mb={3} display="flex" alignItems="center">
        <Typography variant="body1" mr={2} color="text.secondary">Filter by Type:</Typography>
        <Select 
          value={type} 
          onChange={e => setType(e.target.value)} 
          displayEmpty 
          size="small" 
          sx={{ minWidth: 150, bgcolor: 'white' }}
        >
          <MenuItem value="">All Types</MenuItem>
          {['Event','Result','Placement'].map(t => <MenuItem key={t} value={t}>{t}</MenuItem>)}
        </Select>
      </Box>

      {loading && (
        <Box display="flex" justifyContent="center" my={4}>
          <CircularProgress />
        </Box>
      )}
      {error && <Typography color="error" my={2}>{error}</Typography>}
      
      {!loading && !error && data?.map?.((n: any) => (
        <div key={n.id} onClick={() => markViewed(n.id)} style={{ cursor: 'pointer' }}>
          <NotificationCard n={n} viewed={viewed.has(n.id)} />
        </div>
      ))}

      {!loading && !error && data?.length === 0 && (
        <Typography variant="body1" color="text.secondary" align="center" my={4}>
          No notifications found.
        </Typography>
      )}

      {!loading && !error && data?.length > 0 && (
        <Box display="flex" justifyContent="center" mt={4}>
          <Pagination count={10} page={page} onChange={(_, p) => setPage(p)} color="primary" />
        </Box>
      )}
    </Container>
  );
}
