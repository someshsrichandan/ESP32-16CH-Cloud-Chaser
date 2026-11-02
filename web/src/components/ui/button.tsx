'use client'

import * as React from 'react'
import { cva, type VariantProps } from 'class-variance-authority'
import { cn } from '@/lib/utils'

const buttonVariants = cva(
  'inline-flex items-center justify-center whitespace-nowrap rounded-md text-sm font-medium transition-all duration-200 focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-ring disabled:pointer-events-none disabled:opacity-50 active:scale-98 select-none',
  {
    variants: {
      variant: {
        default:
          'bg-cyan-500 text-zinc-950 shadow-md hover:bg-cyan-400 hover:shadow-glow-cyan-sm active:bg-cyan-600 font-semibold',
        destructive:
          'bg-red-600 text-white shadow-md hover:bg-red-500 hover:shadow-glow-red active:bg-red-700',
        outline:
          'border border-zinc-800 bg-zinc-900/60 hover:bg-zinc-800 hover:border-zinc-700 text-zinc-200',
        secondary:
          'bg-zinc-800/80 text-zinc-200 hover:bg-zinc-700/80 active:bg-zinc-800 border border-zinc-700/50',
        ghost: 'hover:bg-zinc-800/70 hover:text-zinc-100 text-zinc-400',
        link: 'text-cyan-400 underline-offset-4 hover:underline',
        cyanGlow:
          'bg-cyan-500/20 text-cyan-300 border border-cyan-500/40 hover:bg-cyan-500/30 hover:border-cyan-400 hover:shadow-glow-cyan-sm',
        amberGlow:
          'bg-amber-500/20 text-amber-300 border border-amber-500/40 hover:bg-amber-500/30 hover:border-amber-400 hover:shadow-glow-amber',
        dangerGlow:
          'bg-red-500/20 text-red-300 border border-red-500/40 hover:bg-red-500/30 hover:border-red-400 hover:shadow-glow-red',
      },
      size: {
        default: 'h-9 px-4 py-2',
        sm: 'h-8 rounded-md px-3 text-xs',
        lg: 'h-11 rounded-md px-8 text-base',
        icon: 'h-9 w-9',
        iconSm: 'h-7 w-7',
      },
    },
    defaultVariants: {
      variant: 'default',
      size: 'default',
    },
  }
)

export interface ButtonProps
  extends React.ButtonHTMLAttributes<HTMLButtonElement>,
    VariantProps<typeof buttonVariants> {
  asChild?: boolean
}

const Button = React.forwardRef<HTMLButtonElement, ButtonProps>(
  ({ className, variant, size, ...props }, ref) => {
    return (
      <button
        className={cn(buttonVariants({ variant, size, className }))}
        ref={ref}
        {...props}
      />
    )
  }
)
Button.displayName = 'Button'

export { Button, buttonVariants }
