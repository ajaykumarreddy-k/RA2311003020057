import { useState } from 'react';
import { useNotifications } from '../hooks/useNotifications';
import NotificationCard from '../components/NotificationCard';
import { AppNotification } from '../types';
import { Loader2, Zap } from 'lucide-react';

const WEIGHTS: Record<string, number> = { Placement: 3, Result: 2, Event: 1 };

function score(n: AppNotification) {
  return (WEIGHTS[n.type] || 1) * (n.is_read ? 1 : 2);
}

export default function Priority() {
  const [topN, setTopN] = useState(10);
  const { data, loading, error } = useNotifications({ limit: 100 });

  const sorted = [...(data || [])]
    .sort((a: AppNotification, b: AppNotification) => score(b) - score(a))
    .slice(0, topN);

  return (
    <div className="max-w-4xl mx-auto px-4 py-8">
      <div className="flex flex-col md:flex-row md:items-center justify-between mb-8 gap-4">
        <div className="flex items-center gap-3">
          <Zap className="text-yellow-500 fill-yellow-500" size={32} />
          <h1 className="text-3xl font-bold text-gray-900 tracking-tight">
            Priority Inbox
          </h1>
        </div>
        
        <div className="flex items-center gap-3 bg-white p-2 rounded-xl shadow-sm border border-gray-100">
          <span className="text-sm font-semibold text-gray-400 ml-2 uppercase tracking-wider">Show Top</span>
          <select 
            value={topN} 
            onChange={e => setTopN(Number(e.target.value))} 
            className="bg-transparent border-none focus:ring-0 text-gray-800 font-bold cursor-pointer py-1 pr-8"
          >
            {[10, 15, 20].map(n => (
              <option key={n} value={n}>{n}</option>
            ))}
          </select>
        </div>
      </div>

      {loading && (
        <div className="flex justify-center items-center py-20">
          <Loader2 className="animate-spin text-blue-600" size={40} />
        </div>
      )}

      {error && (
        <div className="bg-red-50 border border-red-100 text-red-600 p-4 rounded-xl mb-6 font-medium">
          {error}
        </div>
      )}
      
      {!loading && !error && (
        <div className="space-y-1">
          {sorted.map((n: AppNotification) => (
            <NotificationCard key={n.id} n={n} viewed={n.is_read} />
          ))}

          {sorted.length === 0 && (
            <div className="text-center py-20 bg-white rounded-2xl border border-dashed border-gray-200">
              <p className="text-gray-400 font-medium">No high-priority notifications found.</p>
            </div>
          )}
        </div>
      )}
    </div>
  );
}
