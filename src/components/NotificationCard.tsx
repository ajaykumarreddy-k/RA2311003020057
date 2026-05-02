import { AppNotification } from '../types';

const typeColors: Record<string, string> = {
  Placement: 'bg-yellow-100 text-yellow-800',
  Result: 'bg-green-100 text-green-800',
  Event: 'bg-blue-100 text-blue-800',
  default: 'bg-gray-100 text-gray-800'
};

const borderColors: Record<string, string> = {
  active: 'border-l-4 border-blue-600',
  viewed: 'border-l-4 border-gray-400 opacity-70'
};

export default function NotificationCard({ n, viewed }: { n: AppNotification; viewed: boolean }) {
  const colorClass = typeColors[n.type] || typeColors.default;
  const borderClass = viewed ? borderColors.viewed : borderColors.active;

  return (
    <div className={`bg-white rounded-xl shadow-sm p-5 mb-4 border border-gray-100 transition-all hover:shadow-md ${borderClass}`}>
      <div className="flex flex-col gap-2">
        <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium w-fit ${colorClass}`}>
          {n.type}
        </span>
        <p className="text-gray-800 text-base leading-relaxed">
          {n.message}
        </p>
        <span className="text-xs text-gray-500 font-medium">
          {n.timestamp}
        </span>
      </div>
    </div>
  );
}
