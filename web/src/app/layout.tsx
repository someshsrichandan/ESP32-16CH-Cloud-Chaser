import type { Metadata } from 'next'
import './globals.css'
import { Sidebar } from '@/components/ui/sidebar'
import { AppProviders } from './providers'

export const metadata: Metadata = {
  title: 'ESP32 16-CH Relay Chaser Pro',
  description: 'Industrial-grade real-time web controller and sequencer for ESP32 16-channel relay module',
}

export default function RootLayout({
  children,
}: {
  children: React.ReactNode
}) {
  return (
    <html lang="en" className="dark">
      <body className="min-h-screen bg-[#0a0a0f] text-zinc-100 antialiased selection:bg-cyan-500/30 selection:text-cyan-200">
        <AppProviders>
          <div className="flex min-h-screen flex-col md:flex-row">
            <Sidebar />
            <main className="flex-1 overflow-y-auto p-4 md:p-8">
              <div className="mx-auto max-w-7xl">{children}</div>
            </main>
          </div>
        </AppProviders>
      </body>
    </html>
  )
}
