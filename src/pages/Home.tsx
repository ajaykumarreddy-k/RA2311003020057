import { useState } from 'react';
import { useNotifications } from '../hooks/useNotifications';
import NotificationCard from '../components/NotificationCard';
import { AppNotification } from '../types';
import { Loader2, Filter, ChevronLeft, ChevronRight } from 'lucide-react';

export default function Home() {
  const [page, setPage] = useState(1);
  const [type, setType] = useState('');
  const [viewed, setViewed] = useState<Set<string>>(new Set());
  const { data, loading, error } = useNotifications({ limit: 10, page, notification_type: type });

  const markViewed = (id: string) => setViewed(prev => new Set([...prev, id]));

  return (
    <div className="max-w-4xl mx-auto px-4 py-8">
      <div className="flex flex-col md:flex-row md:items-center justify-between mb-8 gap-4">
        <h1 className="text-3xl font-bold text-gray-900 tracking-tight">
          All Notifications
        </h1>
        
        <div className="flex items-center gap-3 bg-white p-2 rounded-xl shadow-sm border border-gray-100">
          <Filter size={18} className="text-gray-400 ml-2" />
          <select 
            value={type} 
            onChange={e => setType(e.target.value)} 
            className="bg-transparent border-none focus:ring-0 text-gray-600 font-medium cursor-pointer py-1 pr-8"
          >
            <option value="">All Types</option>
            {['Event', 'Result', 'Placement'].map(t => (
              <option key={t} value={t}>{t}</option>
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
          {data?.map?.((n: AppNotification) => (
            <div key={n.id} onClick={() => markViewed(n.id)} className="cursor-pointer">
              <NotificationCard n={n} viewed={viewed.has(n.id)} />
            </div>
          ))}

          {data?.length === 0 && (
            <div className="text-center py-20 bg-white rounded-2xl border border-dashed border-gray-200">
              <p className="text-gray-400 font-medium">No notifications found.</p>
            </div>
          )}

          {data?.length > 0 && (
            <div className="flex justify-center items-center gap-2 mt-10">
              <button 
                onClick={() => setPage(p => Math.max(1, p - 1))}
                disabled={page === 1}
                className="p-2 rounded-lg hover:bg-gray-100 disabled:opacity-30 transition-colors"
              >
                <ChevronLeft size={20} />
              </button>
              
              <div className="flex items-center gap-1">
                {[1, 2, 3, 4, 5].map(p => (
                  <button
                    key={p}
                    onClick={() => setPage(p)}
                    className={`w-10 h-10 rounded-lg font-bold transition-all ${
                      page === p 
                        ? 'bg-blue-600 text-white shadow-md shadow-blue-200' 
                        : 'text-gray-600 hover:bg-gray-100'
                    }`}
                  >
                    {p}
                  </button>
                ))}
              </div>

              <button 
                onClick={() => setPage(p => p + 1)}
                className="p-2 rounded-lg hover:bg-gray-100 transition-colors"
              >
                <ChevronRight size={20} />
              </button>
            </div>
          )}
        </div>
      )}
    </div>
  );
}
