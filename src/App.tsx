import { BrowserRouter as Router, Routes, Route, Link, useLocation } from 'react-router-dom';
import { motion, AnimatePresence } from 'motion/react';
import { Bell, LayoutGrid, Award } from 'lucide-react';
import Home from './pages/Home';
import Priority from './pages/Priority';

function Navigation() {
  const location = useLocation();
  
  return (
    <div className="flex justify-center gap-2 mb-12 bg-white p-2 rounded-full shadow-sm border border-gray-100 max-w-fit mx-auto">
      <Link 
        to="/" 
        className={`google-nav-item ${
          location.pathname === '/' 
            ? 'bg-blue-50 text-google-blue' 
            : 'text-gray-500 hover:bg-gray-50'
        }`}
      >
        <LayoutGrid size={20} />
        Notifications
      </Link>
      <Link 
        to="/priority" 
        className={`google-nav-item ${
          location.pathname === '/priority' 
            ? 'bg-blue-50 text-google-blue' 
            : 'text-gray-500 hover:bg-gray-50'
        }`}
      >
        <Award size={20} />
        Priority Inbox
      </Link>
    </div>
  );
}

export default function App() {
  return (
    <Router>
      <div className="min-h-screen bg-google-bg flex flex-col items-center py-16 px-4 sm:px-6">
        <header className="mb-12 text-center">
          <motion.div 
            initial={{ opacity: 0, y: -10 }}
            animate={{ opacity: 1, y: 0 }}
            className="flex justify-center gap-2.5 mb-6"
          >
            <div className="w-3.5 h-3.5 rounded-full bg-google-blue shadow-sm" />
            <div className="w-3.5 h-3.5 rounded-full bg-google-red shadow-sm" />
            <div className="w-3.5 h-3.5 rounded-full bg-google-yellow shadow-sm" />
            <div className="w-3.5 h-3.5 rounded-full bg-google-green shadow-sm" />
          </motion.div>
          <h1 className="text-5xl font-google font-bold text-gray-900 tracking-tighter mb-4">
            Campus Connect
          </h1>
          <p className="text-gray-500 font-medium max-w-md mx-auto text-lg leading-relaxed">
            A Central API to fetch Notifications & Check Notification(According to the Priority).
          </p>
        </header>

        <Navigation />

        <main className="w-full max-w-4xl">
          <AnimatePresence mode="wait">
            <motion.div 
              key={location.pathname}
              initial={{ opacity: 0, scale: 0.98 }}
              animate={{ opacity: 1, scale: 1 }}
              exit={{ opacity: 0, scale: 0.98 }}
              transition={{ duration: 0.2, ease: "easeOut" }}
              className="bg-white rounded-[32px] shadow-[0_8px_30px_rgb(0,0,0,0.04)] border border-gray-100 overflow-hidden min-h-[600px]"
            >
              <Routes>
                <Route path="/" element={<Home />} />
                <Route path="/priority" element={<Priority />} />
              </Routes>
            </motion.div>
          </AnimatePresence>
        </main>
        
        <footer className="mt-20 text-gray-400 text-sm font-google font-bold tracking-widest uppercase">
          &copy; This is Google Inspired website. 
        </footer>
      </div>
    </Router>
  );
}
