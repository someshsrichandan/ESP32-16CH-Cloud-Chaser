'use client'

import * as React from 'react'
import { signIn } from 'next-auth/react'
import { useRouter } from 'next/navigation'
import { Card, CardHeader, CardTitle, CardDescription, CardContent, CardFooter } from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { Input } from '@/components/ui/input'
import { Zap, Lock, Mail, ShieldAlert } from 'lucide-react'
import { toast } from 'sonner'

export default function LoginPage() {
  const router = useRouter()
  const [email, setEmail] = React.useState('admin@relaychaser.local')
  const [password, setPassword] = React.useState('admin')
  const [loading, setLoading] = React.useState(false)

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setLoading(true)

    try {
      const res = await signIn('credentials', {
        redirect: false,
        email,
        password,
      })

      if (res?.error) {
        toast.error('Invalid credentials. Check email and password.')
        setLoading(false)
        return
      }

      toast.success('Access granted. Welcome Master Operator.')
      router.push('/')
      router.refresh()
    } catch (err: any) {
      toast.error(err.message || 'Authentication error')
      setLoading(false)
    }
  }

  return (
    <div className="flex min-h-[80vh] items-center justify-center p-4">
      <Card className="w-full max-w-md border-zinc-800 bg-[#12121a]/95 shadow-2xl">
        <CardHeader className="text-center space-y-2 pb-6">
          <div className="mx-auto flex h-14 w-14 items-center justify-center rounded-2xl bg-gradient-to-br from-cyan-500/20 to-blue-500/10 border border-cyan-500/40 text-cyan-400 shadow-glow-cyan-sm">
            <Zap className="h-7 w-7" />
          </div>
          <CardTitle className="text-xl font-bold tracking-tight text-zinc-100">
            Relay Controller Access
          </CardTitle>
          <CardDescription className="text-zinc-400 text-xs">
            Enter administrative credentials to command the 16-channel ESP32 array.
          </CardDescription>
        </CardHeader>

        <form onSubmit={handleSubmit}>
          <CardContent className="space-y-4">
            <div className="space-y-1.5">
              <label className="text-xs font-mono font-medium text-zinc-300 flex items-center space-x-1.5">
                <Mail className="h-3.5 w-3.5 text-cyan-400" />
                <span>Operator Email</span>
              </label>
              <Input
                type="email"
                required
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                placeholder="admin@relaychaser.local"
                className="bg-zinc-950/60 border-zinc-800 text-sm"
              />
            </div>

            <div className="space-y-1.5">
              <label className="text-xs font-mono font-medium text-zinc-300 flex items-center space-x-1.5">
                <Lock className="h-3.5 w-3.5 text-cyan-400" />
                <span>Password</span>
              </label>
              <Input
                type="password"
                required
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                placeholder="••••••••"
                className="bg-zinc-950/60 border-zinc-800 text-sm"
              />
            </div>

            <div className="rounded-lg border border-amber-500/20 bg-amber-500/10 p-2.5 text-[11px] text-amber-300/90 flex items-start space-x-2">
              <ShieldAlert className="h-4 w-4 shrink-0 text-amber-400 mt-0.5" />
              <span>
                Default credentials prefilled: <strong className="font-mono text-zinc-100">admin@relaychaser.local</strong> / <strong className="font-mono text-zinc-100">admin</strong>
              </span>
            </div>
          </CardContent>

          <CardFooter className="pt-2">
            <Button
              type="submit"
              variant="default"
              disabled={loading}
              className="w-full font-semibold shadow-glow-cyan-sm"
            >
              {loading ? 'Authenticating...' : 'Sign In as Operator'}
            </Button>
          </CardFooter>
        </form>
      </Card>
    </div>
  )
}
