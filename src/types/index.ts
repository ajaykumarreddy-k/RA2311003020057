export interface AppNotification {
  id: string;
  type: 'Placement' | 'Result' | 'Event' | string;
  message: string;
  timestamp: string;
  is_read: boolean;
}
