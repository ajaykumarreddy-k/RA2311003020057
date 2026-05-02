import { BrowserRouter as Router, Routes, Route, Link, useLocation } from 'react-router-dom';
import { motion } from 'motion/react';
import { Bell, ShieldAlert } from 'lucide-react';
import Home from './pages/Home';
import Priority from './pages/Priority';

function Navigation() {
  const location = useLocation();
  
  return (
    <div className="flex justify-center gap-2 mb-10 bg-white p-1.5 rounded-2xl shadow-sm border border-gray-100">
      <Link 
        to="/" 
        className={`px-6 py-2.5 rounded-xl font-bold transition-all flex items-center gap-2 ${
          location.pathname === '/' 
            ? 'bg-blue-600 text-white shadow-lg shadow-blue-100' 
            : 'text-gray-500 hover:bg-gray-50'
        }`}
      >
        <Bell size={18} />
        All Notifications
      </Link>
      <Link 
        to="/priority" 
        className={`px-6 py-2.5 rounded-xl font-bold transition-all flex items-center gap-2 ${
          location.pathname === '/priority' 
            ? 'bg-blue-600 text-white shadow-lg shadow-blue-100' 
            : 'text-gray-500 hover:bg-gray-50'
        }`}
      >
        <ShieldAlert size={18} />
        Priority Inbox
      </Link>
    </div>
  );
}

export default function App() {
  return (
    <Router>
      <div className="min-h-screen bg-[#F8FAFC] flex flex-col items-center py-16 px-4 sm:px-6">
        <header className="mb-10 text-center">
          <div className="inline-flex items-center justify-center w-16 h-16 rounded-2xl bg-blue-600 text-white shadow-xl shadow-blue-100 mb-6">
            <Bell size={32} />
          </div>
          <h1 className="text-4xl font-black text-gray-900 tracking-tight mb-3">
            Campus Notifications
          </h1>
          <p className="text-gray-500 font-medium max-w-md mx-auto">
            Stay updated with the latest events, results, and placement news on campus.
          </p>
        </header>

        <Navigation />

        <main className="w-full max-w-4xl">
          <motion.div 
            initial={{ opacity: 0, y: 20 }}
            animate={{ opacity: 1, y: 0 }}
            className="bg-white rounded-[2rem] shadow-xl shadow-blue-900/5 border border-gray-100 overflow-hidden min-h-[600px]"
          >
            <Routes>
              <Route path="/" element={<Home />} />
              <Route path="/priority" element={<Priority />} />
            </Routes>
          </motion.div>
        </main>
        
        <footer className="mt-20 text-gray-400 text-sm font-medium">
          &copy; 2024 Campus Connect • Notification Service
        </footer>
      </div>
    </Router>
  );
}
