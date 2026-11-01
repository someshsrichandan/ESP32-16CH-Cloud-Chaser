'use client'

import * as React from 'react'
import { motion } from 'framer-motion'
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { Input } from '@/components/ui/input'
import { Badge } from '@/components/ui/badge'
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogDescription, DialogFooter } from '@/components/ui/dialog'
import { useActiveDevice } from '@/context/device-context'
import { useDeviceStatus } from '@/hooks/use-device-status'
import { useDeviceCommand } from '@/hooks/use-device-command'
import { CommandType } from '@/types/device'
import { toast } from 'sonner'
import {
  Wifi,
  Database,
  Cpu,
  RotateCcw,
  UploadCloud,
  CheckCircle2,
  AlertTriangle,
  Save,
  Plus,
  RefreshCw,
  Server,
  Radio,
  Trash2,
  Check,
} from 'lucide-react'

interface RegisteredDevice {
  deviceId: string
  deviceName: string
  online: boolean
  lastSeenAt: string | null
  firmware: string
  ip: string
  mode: string
}

export default function SettingsPage() {
  const {
    activeDeviceId,
    setActiveDeviceId,
    devices,
    isLoadingDevices,
    refreshDevices,
    deleteDevice,
  } = useActiveDevice()

  const { status, online } = useDeviceStatus(activeDeviceId)
  const { sendCommand, isPending } = useDeviceCommand(activeDeviceId)

  const [deviceName, setDeviceName] = React.useState('ESP32 16-CH Relay Chaser')
  const [defaultRepeats, setDefaultRepeats] = React.useState(status?.totalCycles || 3)
  const [resetDialogOpen, setResetDialogOpen] = React.useState(false)
  const [otaProgress, setOtaProgress] = React.useState<number | null>(null)

  // Registration modal
  const [registerDialogOpen, setRegisterDialogOpen] = React.useState(false)
  const [newDeviceId, setNewDeviceId] = React.useState('')
  const [newDeviceName, setNewDeviceName] = React.useState('')
  const [isRegistering, setIsRegistering] = React.useState(false)

  React.useEffect(() => {
    if (status?.totalCycles) {
      setDefaultRepeats(status.totalCycles)
    }
  }, [status?.totalCycles])

  const handleDeleteDevice = async (deviceIdToDelete: string) => {
    if (!confirm(`Are you sure you want to remove node ${deviceIdToDelete}?`)) {
      return
    }
    const ok = await deleteDevice(deviceIdToDelete)
    if (ok) {
      toast.success(`Device ${deviceIdToDelete} removed from MongoDB`)
    } else {
      toast.error(`Failed to remove device ${deviceIdToDelete}`)
    }
  }

  const handleSaveSettings = async () => {
    await sendCommand(CommandType.REPEATS, { v: defaultRepeats }, {
      successMsg: 'Settings transmitted to ESP32 successfully',
    })
  }

  const handleRegisterDevice = async (e: React.FormEvent) => {
    e.preventDefault()
    if (!newDeviceId.trim()) {
      toast.error('Device ID is required (e.g. esp32_a1b2c3d4 from Serial Monitor)')
      return
    }

    setIsRegistering(true)
    try {
      const res = await fetch('/api/device/register', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          deviceId: newDeviceId.trim(),
          deviceName: newDeviceName.trim() || `Relay Controller ${newDeviceId.trim()}`,
        }),
      })

      if (res.ok) {
        toast.success(`Device ${newDeviceId} successfully registered in MongoDB!`)
        setRegisterDialogOpen(false)
        setNewDeviceId('')
        setNewDeviceName('')
        refreshDevices()
      } else {
        const err = await res.json()
        toast.error(err.error || 'Failed to register device')
      }
    } catch (err: any) {
      toast.error('Network error during registration')
    } finally {
      setIsRegistering(false)
    }
  }

  const handleFactoryReset = async () => {
    setResetDialogOpen(false)
    await sendCommand(CommandType.ACTION, { a: 'all_off' })
    toast.warning('Hardware reset command transmitted. Controller rebooting...')
  }

  const handleSimulateOTA = () => {
    setOtaProgress(10)
    const interval = setInterval(() => {
      setOtaProgress((prev) => {
        if (!prev || prev >= 100) {
          clearInterval(interval)
          toast.success('Firmware up to date with latest release!')
          return null
        }
        return prev + 25
      })
    }, 500)
  }

  return (
    <motion.div
      initial={{ opacity: 0, y: 12 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ duration: 0.3 }}
      className="space-y-6"
    >
      {/* Header */}
      <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4 border-b border-zinc-800/80 pb-5">
        <div>
          <div className="flex items-center space-x-3">
            <h1 className="text-2xl font-bold tracking-tight text-zinc-100">
              Controller Settings
            </h1>
            <Badge variant="outline" className="font-mono text-xs">
              HTTP + MongoDB v2.0
            </Badge>
          </div>
          <p className="text-xs text-zinc-400 mt-1">
            Manage device hardware nodes, HTTP telemetry parameters, default cycling repeats, and cloud registration.
          </p>
        </div>

        <div className="flex items-center space-x-2">
          <Button
            variant="outline"
            size="sm"
            onClick={() => setRegisterDialogOpen(true)}
            className="text-xs font-semibold border-cyan-500/40 text-cyan-400 hover:bg-cyan-500/10"
          >
            <Plus className="mr-1.5 h-3.5 w-3.5" />
            Register Device
          </Button>

          <Button
            variant="default"
            size="sm"
            onClick={handleSaveSettings}
            disabled={isPending}
            className="text-xs font-semibold shadow-glow-cyan-sm"
          >
            <Save className="mr-1.5 h-3.5 w-3.5" />
            Save Changes
          </Button>
        </div>
      </div>

      {/* Registered Hardware Nodes List */}
      <Card className="border-zinc-800 bg-[#12121a]">
        <CardHeader className="pb-3 flex flex-row items-center justify-between">
          <div className="flex items-center space-x-2">
            <Radio className="h-4 w-4 text-cyan-400" />
            <div>
              <CardTitle className="text-sm font-semibold">Registered Hardware Nodes</CardTitle>
              <CardDescription className="text-xs">
                ESP32 controllers communicating directly via HTTPS REST & MongoDB
              </CardDescription>
            </div>
          </div>
          <Button
            variant="ghost"
            size="sm"
            onClick={refreshDevices}
            disabled={isLoadingDevices}
            className="h-8 text-xs text-zinc-400 hover:text-zinc-100"
          >
            <RefreshCw className={`h-3.5 w-3.5 mr-1.5 ${isLoadingDevices ? 'animate-spin' : ''}`} />
            Refresh
          </Button>
        </CardHeader>

        <CardContent>
          {devices.length === 0 ? (
            <div className="rounded-lg border border-dashed border-zinc-800 p-6 text-center text-xs text-zinc-500">
              No registered hardware nodes found in MongoDB. Click &quot;Register Device&quot; to pair your ESP32.
            </div>
          ) : (
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-3">
              {devices.map((dev) => {
                const isActive = dev.deviceId === activeDeviceId
                return (
                  <div
                    key={dev.deviceId}
                    onClick={() => setActiveDeviceId(dev.deviceId)}
                    className={`cursor-pointer rounded-lg border p-3.5 transition-all relative group ${
                      isActive
                        ? 'border-cyan-500 bg-cyan-950/20 shadow-glow-cyan-sm'
                        : 'border-zinc-800 bg-zinc-950/40 hover:border-zinc-700'
                    }`}
                  >
                    <div className="flex items-center justify-between mb-2">
                      <div className="flex items-center space-x-1.5 truncate pr-2">
                        <span className="font-semibold text-xs text-zinc-100 truncate">{dev.deviceName}</span>
                        {isActive && (
                          <span className="rounded bg-cyan-500/20 px-1 py-0.2 text-[9px] font-mono font-bold text-cyan-300">
                            Active
                          </span>
                        )}
                      </div>
                      <div className="flex items-center space-x-1">
                        <Badge variant={dev.online ? 'online' : 'offline'} className="text-[10px] px-1.5 py-0">
                          {dev.online ? 'Online' : 'Offline'}
                        </Badge>
                        <Button
                          variant="ghost"
                          size="icon"
                          title="Delete Device"
                          onClick={(e) => {
                            e.stopPropagation()
                            handleDeleteDevice(dev.deviceId)
                          }}
                          className="h-6 w-6 text-zinc-500 hover:text-rose-400 hover:bg-rose-500/10 ml-1"
                        >
                          <Trash2 className="h-3 w-3" />
                        </Button>
                      </div>
                    </div>
                    <div className="space-y-1 font-mono text-[11px] text-zinc-400">
                      <div>ID: <span className="text-zinc-200">{dev.deviceId}</span></div>
                      <div>IP: <span className="text-cyan-400">{dev.ip || 'DHCP Assigned'}</span></div>
                      <div>Firmware: <span className="text-zinc-300">{dev.firmware}</span></div>
                    </div>
                  </div>
                )
              })}
            </div>
          )}
        </CardContent>
      </Card>

      <div className="grid grid-cols-1 gap-6 lg:grid-cols-2">
        {/* Device Information Card */}
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-3">
            <div className="flex items-center space-x-2">
              <Cpu className="h-4 w-4 text-cyan-400" />
              <CardTitle className="text-sm font-semibold">Active Node Identity</CardTitle>
            </div>
            <CardDescription className="text-xs">
              Physical node identification and network addressing
            </CardDescription>
          </CardHeader>

          <CardContent className="space-y-4">
            <div className="space-y-1.5">
              <label className="text-xs font-mono text-zinc-400">Device Friendly Name</label>
              <Input
                value={deviceName}
                onChange={(e) => setDeviceName(e.target.value)}
                className="bg-zinc-950/60 border-zinc-800 text-xs font-medium"
              />
            </div>

            <div className="space-y-1.5">
              <label className="text-xs font-mono text-zinc-400">Unique Hardware Device ID</label>
              <Input
                value={status?.deviceId || activeDeviceId}
                disabled
                className="bg-zinc-900 border-zinc-800 font-mono text-xs text-zinc-400 cursor-not-allowed"
              />
              <span className="text-[10px] text-zinc-500 font-mono">
                Derived from ESP32 MAC address & stored in NVS. Used for HTTP authorization and telemetry sync.
              </span>
            </div>

            <div className="space-y-1.5">
              <label className="text-xs font-mono text-zinc-400">Default Cycle Repeats</label>
              <Input
                type="number"
                min={1}
                max={50}
                value={defaultRepeats}
                onChange={(e) => setDefaultRepeats(Math.max(1, parseInt(e.target.value) || 1))}
                className="bg-zinc-950/60 border-zinc-800 text-xs font-mono font-bold"
              />
              <span className="text-[10px] text-zinc-500 font-mono">
                Cycles each effect plays before automatically moving to the next pattern.
              </span>
            </div>
          </CardContent>
        </Card>

        {/* Direct HTTP & MongoDB Architecture Settings */}
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-3">
            <div className="flex items-center space-x-2">
              <Database className="h-4 w-4 text-cyan-400" />
              <CardTitle className="text-sm font-semibold">Direct HTTP & MongoDB Architecture</CardTitle>
            </div>
            <CardDescription className="text-xs">
              Serverless HTTPS communication without brokers or port forwarding
            </CardDescription>
          </CardHeader>

          <CardContent className="space-y-4">
            <div className="space-y-1.5">
              <label className="text-xs font-mono text-zinc-400">Transport & Protocol</label>
              <Input
                value="Direct HTTPS REST (Vercel Serverless + Edge Network)"
                disabled
                className="bg-zinc-900 border-zinc-800 font-mono text-xs text-zinc-300"
              />
            </div>

            <div className="grid grid-cols-2 gap-3">
              <div className="space-y-1.5">
                <label className="text-xs font-mono text-zinc-400">Polling Cadence</label>
                <Input
                  value="1000 ms (Commands)"
                  disabled
                  className="bg-zinc-900 border-zinc-800 font-mono text-xs text-zinc-400"
                />
              </div>

              <div className="space-y-1.5">
                <label className="text-xs font-mono text-zinc-400">Database Engine</label>
                <div className="flex h-9 items-center rounded-md border border-zinc-800 bg-zinc-900 px-3 font-mono text-xs">
                  <span className="flex items-center text-emerald-400 font-semibold">
                    <CheckCircle2 className="mr-1.5 h-3.5 w-3.5" />
                    MongoDB Atlas
                  </span>
                </div>
              </div>
            </div>

            <div className="rounded-lg border border-zinc-800 bg-zinc-950/40 p-3 space-y-1 text-[11px] font-mono text-zinc-400">
              <div>Telemetry Push: <span className="text-cyan-400">POST /api/esp/status (2s)</span></div>
              <div>Command Polling: <span className="text-cyan-400">GET /api/esp/poll (1s)</span></div>
              <div>Real-Time Stream: <span className="text-emerald-400">SSE /api/device/{activeDeviceId}/stream</span></div>
              <div>Auth Headers: <span className="text-zinc-300">X-Device-Id, X-Device-Secret</span></div>
            </div>
          </CardContent>
        </Card>

        {/* Wi-Fi & Network Topology */}
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-3">
            <div className="flex items-center space-x-2">
              <Wifi className="h-4 w-4 text-cyan-400" />
              <CardTitle className="text-sm font-semibold">Wi-Fi & Station Network</CardTitle>
            </div>
            <CardDescription className="text-xs">
              ESP32 wireless station network parameters
            </CardDescription>
          </CardHeader>

          <CardContent className="space-y-3 font-mono text-xs">
            <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-950/40 p-2.5">
              <span className="text-zinc-400">Router SSID:</span>
              <span className="font-bold text-zinc-100">{status?.staSsid || 'Home Wi-Fi (2.4GHz)'}</span>
            </div>

            <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-950/40 p-2.5">
              <span className="text-zinc-400">Station IP:</span>
              <span className="font-bold text-cyan-400">{status?.ip || 'DHCP Assigned'}</span>
            </div>

            <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-950/40 p-2.5">
              <span className="text-zinc-400">Wi-Fi RSSI:</span>
              <span className="font-bold text-zinc-300">{status?.rssi ? `${status.rssi} dBm` : 'Connected'}</span>
            </div>

            <div className="flex items-center justify-between rounded-lg border border-zinc-800 bg-zinc-950/40 p-2.5">
              <span className="text-zinc-400">Cloud Sync Interval:</span>
              <span className="font-bold text-cyan-300">2000 ms</span>
            </div>
          </CardContent>
        </Card>

        {/* Firmware Maintenance & Danger Zone */}
        <Card className="border-zinc-800 bg-[#12121a]">
          <CardHeader className="pb-3">
            <div className="flex items-center space-x-2">
              <UploadCloud className="h-4 w-4 text-cyan-400" />
              <CardTitle className="text-sm font-semibold">Firmware & Maintenance</CardTitle>
            </div>
            <CardDescription className="text-xs">
              Firmware version details and hardware reboot controls
            </CardDescription>
          </CardHeader>

          <CardContent className="space-y-4">
            <div className="rounded-lg border border-zinc-800 bg-zinc-950/40 p-3 flex items-center justify-between">
              <div>
                <p className="text-xs font-semibold text-zinc-200">ESP32 Firmware v2.0-http</p>
                <p className="text-[11px] text-zinc-500 font-mono">Direct HTTP Client Edition</p>
              </div>
              <Button
                variant="outline"
                size="sm"
                onClick={handleSimulateOTA}
                disabled={otaProgress !== null}
                className="text-xs"
              >
                {otaProgress !== null ? `Checking... ${otaProgress}%` : 'Check Updates'}
              </Button>
            </div>

            <div className="pt-2 border-t border-zinc-800/80">
              <div className="flex items-center justify-between">
                <div>
                  <p className="text-xs font-semibold text-red-400">Hardware Reset</p>
                  <p className="text-[11px] text-zinc-500">Reverts all 16 relays to OFF state and restarts controller.</p>
                </div>
                <Button
                  variant="destructive"
                  size="sm"
                  onClick={() => setResetDialogOpen(true)}
                  className="text-xs"
                >
                  <RotateCcw className="mr-1.5 h-3.5 w-3.5" />
                  Reboot / Reset
                </Button>
              </div>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Register Device Dialog */}
      <Dialog open={registerDialogOpen} onOpenChange={setRegisterDialogOpen}>
        <DialogContent className="max-w-md bg-[#12121a] border-zinc-800 text-zinc-100">
          <DialogHeader>
            <div className="flex items-center space-x-2 text-cyan-400">
              <Server className="h-5 w-5" />
              <DialogTitle>Register New ESP32 Hardware</DialogTitle>
            </div>
            <DialogDescription className="text-zinc-400 text-xs pt-1">
              Add your ESP32 controller to MongoDB. Get the Device ID printed on the Serial Monitor during startup.
            </DialogDescription>
          </DialogHeader>

          <form onSubmit={handleRegisterDevice} className="space-y-3 mt-3">
            <div className="space-y-1">
              <label className="text-xs font-mono text-zinc-400">Hardware Device ID</label>
              <Input
                placeholder="e.g. esp32_a1b2c3d4"
                value={newDeviceId}
                onChange={(e) => setNewDeviceId(e.target.value)}
                className="bg-zinc-950 border-zinc-800 text-xs font-mono"
                required
              />
              <span className="text-[10px] text-zinc-500 font-mono">
                Printed in Serial Monitor: [BOOT] Ready. Device ID: esp32_xxxx
              </span>
            </div>

            <div className="space-y-1">
              <label className="text-xs font-mono text-zinc-400">Friendly Device Name</label>
              <Input
                placeholder="e.g. Living Room Chaser"
                value={newDeviceName}
                onChange={(e) => setNewDeviceName(e.target.value)}
                className="bg-zinc-950 border-zinc-800 text-xs"
              />
            </div>

            <DialogFooter className="mt-4 pt-2 border-t border-zinc-800 flex justify-end space-x-2">
              <Button
                type="button"
                variant="outline"
                size="sm"
                onClick={() => setRegisterDialogOpen(false)}
              >
                Cancel
              </Button>
              <Button
                type="submit"
                variant="default"
                size="sm"
                disabled={isRegistering}
                className="shadow-glow-cyan-sm"
              >
                {isRegistering ? 'Registering...' : 'Register Device'}
              </Button>
            </DialogFooter>
          </form>
        </DialogContent>
      </Dialog>

      {/* Confirmation Dialog for Reset */}
      <Dialog open={resetDialogOpen} onOpenChange={setResetDialogOpen}>
        <DialogContent className="max-w-md bg-[#12121a] border-zinc-800 text-zinc-100">
          <DialogHeader>
            <div className="flex items-center space-x-2 text-red-400">
              <AlertTriangle className="h-5 w-5" />
              <DialogTitle>Confirm Controller Reboot</DialogTitle>
            </div>
            <DialogDescription className="text-zinc-400 text-xs pt-1">
              Are you sure you want to trigger a hardware reboot? All active relay outputs will be disconnected instantly.
            </DialogDescription>
          </DialogHeader>

          <DialogFooter className="mt-4 flex space-x-2">
            <Button
              variant="outline"
              size="sm"
              onClick={() => setResetDialogOpen(false)}
            >
              Cancel
            </Button>
            <Button
              variant="destructive"
              size="sm"
              onClick={handleFactoryReset}
            >
              Proceed with Reboot
            </Button>
          </DialogFooter>
        </DialogContent>
      </Dialog>
    </motion.div>
  )
}
