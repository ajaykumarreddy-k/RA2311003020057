import { AppNotification } from '../types';

const typeStyles: Record<string, string> = {
  Placement: 'bg-blue-50 text-google-blue',
  Result: 'bg-green-50 text-google-green',
  Event: 'bg-red-50 text-google-red',
  default: 'bg-gray-50 text-gray-500'
};

const statusStyles: Record<string, string> = {
  active: 'border-l-[6px] border-google-blue',
  viewed: 'border-l-[6px] border-gray-300 opacity-60'
};

export default function NotificationCard({ n, viewed }: { n: AppNotification; viewed: boolean }) {
  const badgeStyle = typeStyles[n.type] || typeStyles.default;
  const statusStyle = viewed ? statusStyles.viewed : statusStyles.active;

  return (
    <div className={`google-card mb-4 border-l-0 ${statusStyle}`}>
      <div className="flex flex-col gap-3">
        <div className="flex items-center justify-between">
          <span className={`px-3 py-1 rounded-lg text-xs font-bold tracking-wider uppercase ${badgeStyle}`}>
            {n.type}
          </span>
          <span className="text-[11px] font-bold text-gray-400 uppercase tracking-widest">
            {n.timestamp}
          </span>
        </div>
        <p className="text-gray-800 text-lg font-google font-medium leading-snug">
          {n.message}
        </p>
      </div>
    </div>
  );
}
