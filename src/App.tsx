import { BrowserRouter as Router, Routes, Route, Link, useLocation } from 'react-router-dom';
import { motion } from 'motion/react';
import { Bell, ShieldAlert } from 'lucide-react';
import Home from './pages/Home';
import Priority from './pages/Priority';

function Navigation() {
  const location = useLocation();
  
  return (
    <div className="flex justify-center gap-4 mb-8">
      <Link 
        to="/" 
        className={`px-4 py-2 rounded-full font-medium transition-all flex items-center gap-2 ${
          location.pathname === '/' 
            ? 'bg-blue-100 text-google-blue' 
            : 'text-gray-500 hover:bg-gray-100'
        }`}
      >
        <Bell size={18} />
        All Notifications
      </Link>
      <Link 
        to="/priority" 
        className={`px-4 py-2 rounded-full font-medium transition-all flex items-center gap-2 ${
          location.pathname === '/priority' 
            ? 'bg-red-100 text-google-red' 
            : 'text-gray-500 hover:bg-gray-100'
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
      <div id="google-theme-app" className="min-h-screen bg-gray-50 flex flex-col items-center py-12 px-4 sm:px-6">
        <header className="mb-8 text-center">
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
            Campus Notifications
          </h1>
          <p className="text-gray-500 font-medium">Simple, clean, and Material inspired</p>
        </header>

        <Navigation />

        <main className="w-full max-w-4xl">
          <motion.div 
            initial={{ opacity: 0, y: 20 }}
            animate={{ opacity: 1, y: 0 }}
            className="bg-white rounded-3xl shadow-sm border border-gray-100 p-6 min-h-[500px]"
          >
            <Routes>
              <Route path="/" element={<Home />} />
              <Route path="/priority" element={<Priority />} />
            </Routes>
          </motion.div>
        </main>

        <footer className="mt-auto pt-12 pb-4 text-gray-400 text-xs flex items-center gap-1">
          <span>Built with</span>
          <div className="w-2 h-2 rounded-full bg-google-blue mx-0.5" />
          <div className="w-2 h-2 rounded-full bg-google-red mx-0.5" />
          <div className="w-2 h-2 rounded-full bg-google-yellow mx-0.5" />
          <div className="w-2 h-2 rounded-full bg-google-green mx-0.5" />
          <span className="ml-1 tracking-wider uppercase font-semibold">Google Aesthetics</span>
        </footer>
      </div>
    </Router>
  );
}
