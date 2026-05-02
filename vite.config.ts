import tailwindcss from '@tailwindcss/vite';
import react from '@vitejs/plugin-react';
import path from 'path';
import {defineConfig, loadEnv} from 'vite';

export default defineConfig(({mode}) => {
  const env = loadEnv(mode, '.', '');
  return {
    plugins: [react(), tailwindcss()],
    define: {
      'process.env.GEMINI_API_KEY': JSON.stringify(env.GEMINI_API_KEY),
      'process.env.TARGET_API_KEY': JSON.stringify(env.TARGET_API_KEY),
    },
    resolve: {
      alias: {
        '@': path.resolve(__dirname, '.'),
        'logging-middleware': path.resolve(__dirname, './logging_middleware/src/index.ts'),
        'logging-middleware/dist/types': path.resolve(__dirname, './logging_middleware/src/types.ts')
      },
    },
    server: {
      // HMR is disabled in AI Studio via DISABLE_HMR env var.
      // Do not modify—file watching is disabled to prevent flickering during agent edits.
      hmr: process.env.DISABLE_HMR !== 'true',
      proxy: {
        '/evaluation-service': {
          target: 'http://20.207.122.201',
          changeOrigin: true,
          secure: false,
        }
      }
    },
  };
});
