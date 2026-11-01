'use client'

import * as React from 'react'
import { SessionProvider } from 'next-auth/react'
import { Toaster } from 'sonner'
import { TooltipProvider } from '@/components/ui/tooltip'

export function AppProviders({ children }: { children: React.ReactNode }) {
  return (
    <SessionProvider>
      <TooltipProvider delayDuration={150}>
        {children}
        <Toaster
          position="bottom-right"
          theme="dark"
          toastOptions={{
            style: {
              background: '#12121a',
              border: '1px solid rgba(39, 39, 42, 0.8)',
              color: '#f4f4f5',
            },
          }}
        />
      </TooltipProvider>
    </SessionProvider>
  )
}
