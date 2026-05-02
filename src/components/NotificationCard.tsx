import { Card, CardContent, Chip, Typography } from '@mui/material';

const typeColors: Record<string, 'primary'|'success'|'warning'> = {
  Placement: 'warning', Result: 'success', Event: 'primary'
};

export default function NotificationCard({ n, viewed }: { n: any; viewed: boolean }) {
  return (
    <Card sx={{ mb: 2, opacity: viewed ? 0.7 : 1, borderLeft: viewed ? '4px solid gray' : '4px solid #1976d2' }}>
      <CardContent>
        <Chip label={n.type} color={typeColors[n.type] || 'default'} size="small" />
        <Typography variant="body1" mt={1}>{n.message}</Typography>
        <Typography variant="caption" color="text.secondary">{n.timestamp}</Typography>
      </CardContent>
    </Card>
  );
}
