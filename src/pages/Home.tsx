import { useState } from 'react';
import { useNotifications } from '../hooks/useNotifications';
import NotificationCard from '../components/NotificationCard';
import { AppNotification } from '../types';
import { Loader2, ChevronLeft, ChevronRight, ListFilter } from 'lucide-react';

export default function Home() {
  const [page, setPage] = useState(1);
  const [type, setType] = useState('');
  const [viewed, setViewed] = useState<Set<string>>(new Set());
  const { data, loading, error } = useNotifications({ limit: 10, page, notification_type: type });

  const markViewed = (id: string) => setViewed(prev => new Set([...prev, id]));

  return (
    <div className="p-8 md:p-12">
      <div className="flex flex-col md:flex-row md:items-center justify-between mb-10 gap-6">
        <h2 className="text-3xl font-google font-bold text-gray-900 tracking-tight">
          All Notifications
        </h2>
        
        <div className="flex items-center gap-3 bg-google-bg px-4 py-2 rounded-xl border border-gray-200">
          <ListFilter size={18} className="text-gray-400" />
          <select 
            value={type} 
            onChange={e => { setType(e.target.value); setPage(1); }} 
            className="bg-transparent border-none focus:ring-0 text-sm font-bold text-gray-600 cursor-pointer outline-none"
          >
            <option value="">Filter by Type</option>
            {['Event', 'Result', 'Placement'].map(t => (
              <option key={t} value={t}>{t}</option>
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
          {data?.map?.((n: AppNotification) => (
            <div key={n.id} onClick={() => markViewed(n.id)} className="cursor-pointer active:scale-[0.99] transition-transform">
              <NotificationCard n={n} viewed={viewed.has(n.id)} />
            </div>
          ))}

          {data?.length === 0 && (
            <div className="text-center py-24 bg-google-bg rounded-3xl border-2 border-dashed border-gray-200">
              <p className="text-gray-400 font-google font-bold text-lg">No notifications found</p>
            </div>
          )}

          {data?.length > 0 && (
            <div className="flex justify-center items-center gap-3 mt-12">
              <button 
                onClick={() => setPage(p => Math.max(1, p - 1))}
                disabled={page === 1}
                className="w-12 h-12 flex items-center justify-center rounded-2xl bg-white border border-gray-200 text-gray-400 hover:text-google-blue hover:border-google-blue disabled:opacity-20 transition-all shadow-sm"
              >
                <ChevronLeft size={24} />
              </button>
              
              <div className="flex items-center gap-2">
                {[1, 2, 3].map(p => (
                  <button
                    key={p}
                    onClick={() => setPage(p)}
                    className={`w-12 h-12 rounded-2xl font-google font-bold text-lg transition-all shadow-sm border ${
                      page === p 
                        ? 'bg-google-blue text-white border-google-blue shadow-blue-100' 
                        : 'bg-white text-gray-600 border-gray-200 hover:border-google-blue hover:text-google-blue'
                    }`}
                  >
                    {p}
                  </button>
                ))}
              </div>

              <button 
                onClick={() => setPage(p => p + 1)}
                className="w-12 h-12 flex items-center justify-center rounded-2xl bg-white border border-gray-200 text-gray-400 hover:text-google-blue hover:border-google-blue transition-all shadow-sm"
              >
                <ChevronRight size={24} />
              </button>
            </div>
          )}
        </div>
      )}
    </div>
  );
}
