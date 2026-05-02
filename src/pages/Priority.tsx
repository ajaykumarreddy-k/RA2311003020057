import { useState } from 'react';
import { useNotifications } from '../hooks/useNotifications';
import NotificationCard from '../components/NotificationCard';
import { AppNotification } from '../types';
import { Loader2 } from 'lucide-react';
import priorityIcon from '../../Assets /Priority.gif';

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
    <div className="p-8 md:p-12">
      <div className="flex flex-col md:flex-row md:items-center justify-between mb-10 gap-6">
        <div className="flex items-center gap-4">
          <div className="w-12 h-12 rounded-2xl bg-google-yellow flex items-center justify-center shadow-lg shadow-yellow-100 overflow-hidden">
            <img src={priorityIcon} alt="Priority" className="w-full h-full object-cover" />
          </div>
          <h2 className="text-3xl font-google font-bold text-gray-900 tracking-tight">
            Priority Inbox
          </h2>
        </div>
        
        <div className="flex items-center gap-3 bg-google-bg px-4 py-2 rounded-xl border border-gray-200">
          <span className="text-xs font-bold text-gray-400 uppercase tracking-widest ml-1">Rank Top</span>
          <select 
            value={topN} 
            onChange={e => setTopN(Number(e.target.value))} 
            className="bg-transparent border-none focus:ring-0 text-sm font-bold text-google-blue cursor-pointer outline-none"
          >
            {[10, 15, 20].map(n => (
              <option key={n} value={n}>{n}</option>
            ))}
          </select>
        </div>
      </div>

      {loading && (
        <div className="flex justify-center items-center py-32">
          <Loader2 className="animate-spin text-google-blue" size={48} strokeWidth={1.5} />
        </div>
      )}

      {error && (
        <div className="bg-red-50 border border-red-100 text-google-red p-5 rounded-2xl mb-8 font-google font-bold text-center">
          {error}
        </div>
      )}
      
      {!loading && !error && (
        <div className="space-y-4">
          {sorted.map((n: AppNotification) => (
            <NotificationCard key={n.id} n={n} viewed={n.is_read} />
          ))}

          {sorted.length === 0 && (
            <div className="text-center py-24 bg-google-bg rounded-3xl border-2 border-dashed border-gray-200">
              <p className="text-gray-400 font-google font-bold text-lg">No high-priority notifications</p>
            </div>
          )}
        </div>
      )}
    </div>
  );
}
