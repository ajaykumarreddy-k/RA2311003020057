import { useState } from 'react';
import { motion, AnimatePresence } from 'motion/react';
import { Search, Globe, Send, Terminal, Loader2, RefreshCcw, ShieldCheck, Mail } from 'lucide-react';

export default function App() {
  const [url, setUrl] = useState('https://jsonplaceholder.typicode.com/posts/1');
  const [method, setMethod] = useState('GET');
  const [response, setResponse] = useState<any>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const handleTestApi = async () => {
    setLoading(true);
    setError(null);
    setResponse(null);
    try {
      const startTime = performance.now();
      const res = await fetch(url, { method });
      const data = await res.json();
      const endTime = performance.now();
      
      setResponse({
        status: res.status,
        statusText: res.statusText,
        time: `${(endTime - startTime).toFixed(2)}ms`,
        data
      });
    } catch (err: any) {
      setError(err.message || 'Failed to fetch API');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div id="google-theme-app" className="min-h-screen bg-gray-50 flex flex-col items-center py-12 px-4 sm:px-6">
      <header className="mb-12 text-center">
        <motion.div 
          initial={{ opacity: 0, y: -20 }}
          animate={{ opacity: 1, y: 0 }}
          className="flex justify-center gap-2 mb-4"
        >
          <div className="w-4 h-4 rounded-full bg-google-blue" />
          <div className="w-4 h-4 rounded-full bg-google-red" />
          <div className="w-4 h-4 rounded-full bg-google-yellow" />
          <div className="w-4 h-4 rounded-full bg-google-green" />
        </motion.div>
        <h1 className="text-4xl font-google font-bold text-gray-800 tracking-tight mb-2">
          Google Theme API
        </h1>
        <p className="text-gray-500 font-medium">Simple, clean, and Material inspired</p>
      </header>

      <main className="w-full max-w-3xl space-y-6">
        {/* Request Tile */}
        <section id="request-section" className="google-tile">
          <div className="flex items-center gap-3 mb-6">
            <div className="w-10 h-10 rounded-full bg-blue-50 flex items-center justify-center text-google-blue">
              <Globe size={20} />
            </div>
            <div>
              <h2 className="text-lg font-semibold text-gray-800">API Configuration</h2>
              <p className="text-xs text-gray-500">Configure your endpoint and method</p>
            </div>
          </div>

          <div className="space-y-4">
            <div className="flex flex-col sm:flex-row gap-3">
              <select 
                value={method}
                onChange={(e) => setMethod(e.target.value)}
                className="px-4 py-3 rounded-google-sm border border-gray-300 bg-white font-medium focus:border-google-blue outline-none cursor-pointer"
              >
                <option>GET</option>
                <option>POST</option>
                <option>PUT</option>
                <option>DELETE</option>
              </select>
              <div className="flex-1 relative">
                <input 
                  type="text" 
                  value={url} 
                  onChange={(e) => setUrl(e.target.value)}
                  placeholder="Enter API URL..."
                  className="google-input pl-10"
                />
                <Search className="absolute left-3 top-3.5 text-gray-400" size={18} />
              </div>
            </div>

            <div className="flex justify-between items-center pt-2">
              <div className="flex items-center gap-2">
                <div className={`w-2 h-2 rounded-full ${process.env.TARGET_API_KEY ? 'bg-google-green' : 'bg-gray-300'}`} />
                <span className="text-[10px] text-gray-400 uppercase font-bold tracking-wider">
                  Target API Key: {process.env.TARGET_API_KEY ? 'Configured' : 'Missing'}
                </span>
              </div>
              <button 
                onClick={handleTestApi}
                disabled={loading || !url}
                className="google-button-primary flex items-center gap-2"
              >
                {loading ? <Loader2 className="animate-spin" size={18} /> : <Send size={18} />}
                {loading ? 'Testing...' : 'Test Request'}
              </button>
            </div>
          </div>
        </section>

        {/* Info/Features Tile */}
        <section id="features-section" className="grid grid-cols-1 md:grid-cols-3 gap-6">
          <div className="google-tile p-4 flex flex-col items-center text-center">
            <ShieldCheck className="text-google-green mb-2" size={24} />
            <h3 className="font-semibold text-sm">Secure</h3>
            <p className="text-xs text-gray-500 mt-1">Direct browser-to-API calls</p>
          </div>
          <div className="google-tile p-4 flex flex-col items-center text-center">
            <RefreshCcw className="text-google-yellow mb-2" size={24} />
            <h3 className="font-semibold text-sm">Real-time</h3>
            <p className="text-xs text-gray-500 mt-1">Instant feedback on responses</p>
          </div>
          <div className="google-tile p-4 flex flex-col items-center text-center">
            <Mail className="text-google-red mb-2" size={24} />
            <h3 className="font-semibold text-sm">Support</h3>
            <p className="text-xs text-gray-500 mt-1">Material Design components</p>
          </div>
        </section>

        {/* Result Tile */}
        <AnimatePresence mode="wait">
          {(response || error) && (
            <motion.section 
              id="result-section"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, scale: 0.95 }}
              className="google-tile overflow-hidden"
            >
              <div className="flex items-center justify-between mb-4 pb-4 border-b border-gray-100">
                <div className="flex items-center gap-3">
                  <div className={`w-10 h-10 rounded-full flex items-center justify-center ${error ? 'bg-red-50 text-google-red' : 'bg-green-50 text-google-green'}`}>
                    <Terminal size={20} />
                  </div>
                  <div>
                    <h2 className="text-lg font-semibold text-gray-800">{error ? 'Error' : 'Response'}</h2>
                    {!error && <p className="text-xs text-gray-500">Status: {response.status} • Time: {response.time}</p>}
                  </div>
                </div>
                {!error && (
                  <span className={`px-3 py-1 rounded-full text-xs font-bold ${response.status >= 200 && response.status < 300 ? 'bg-green-100 text-google-green' : 'bg-yellow-100 text-google-yellow'}`}>
                    {response.statusText}
                  </span>
                )}
              </div>

              {error ? (
                <div className="bg-red-50 p-4 rounded-google-sm border border-red-100 text-google-red text-sm font-medium">
                  {error}
                </div>
              ) : (
                <div className="relative group">
                  <pre className="bg-gray-900 rounded-google-sm p-4 text-google-green text-sm overflow-x-auto font-mono scrollbar-thin scrollbar-thumb-gray-700">
                    <code>{JSON.stringify(response.data, null, 2)}</code>
                  </pre>
                  <div className="absolute top-2 right-2 opacity-0 group-hover:opacity-100 transition-opacity">
                    <button 
                      onClick={() => navigator.clipboard.writeText(JSON.stringify(response.data, null, 2))}
                      className="p-1.5 bg-gray-800 text-white rounded hover:bg-gray-700 text-[10px]"
                    >
                      Copy
                    </button>
                  </div>
                </div>
              )}
            </motion.section>
          )}
        </AnimatePresence>
      </main>

      <footer className="mt-auto py-8 text-gray-400 text-xs flex items-center gap-1">
        <span>Built with</span>
        <div className="w-2 h-2 rounded-full bg-google-blue mx-0.5" />
        <div className="w-2 h-2 rounded-full bg-google-red mx-0.5" />
        <div className="w-2 h-2 rounded-full bg-google-yellow mx-0.5" />
        <div className="w-2 h-2 rounded-full bg-google-green mx-0.5" />
        <span className="ml-1 tracking-wider uppercase font-semibold">Google Aesthetics</span>
      </footer>
    </div>
  );
}
