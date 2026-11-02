import * as React from 'react'
import { cva, type VariantProps } from 'class-variance-authority'
import { cn } from '@/lib/utils'

const badgeVariants = cva(
  'inline-flex items-center rounded-full border px-2.5 py-0.5 text-xs font-semibold transition-colors focus:outline-none focus:ring-2 focus:ring-ring focus:ring-offset-2',
  {
    variants: {
      variant: {
        default: 'border-transparent bg-cyan-500/20 text-cyan-300 border-cyan-500/40',
        secondary: 'border-transparent bg-zinc-800 text-zinc-300',
        destructive: 'border-transparent bg-red-500/20 text-red-300 border-red-500/40',
        outline: 'text-zinc-300 border-zinc-700',
        success: 'border-emerald-500/40 bg-emerald-500/20 text-emerald-300',
        warning: 'border-amber-500/40 bg-amber-500/20 text-amber-300',
        online: 'border-emerald-500/40 bg-emerald-500/20 text-emerald-300 shadow-glow-green',
        offline: 'border-zinc-700 bg-zinc-800/80 text-zinc-400',
      },
    },
    defaultVariants: {
      variant: 'default',
    },
  }
)

export interface BadgeProps
  extends React.HTMLAttributes<HTMLDivElement>,
    VariantProps<typeof badgeVariants> {}

function Badge({ className, variant, ...props }: BadgeProps) {
  return <div className={cn(badgeVariants({ variant }), className)} {...props} />
}

export { Badge, badgeVariants }
