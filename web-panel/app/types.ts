export interface DeviceState {
  device_id: string;
  e: number;               // Current effect index (0..40)
  ename: string;           // Current effect name
  s: number;               // Speed (ms)
  a: number;               // Auto mode (1 or 0)
  sh: number;              // Auto shuffle (1 or 0)
  r: number;               // Running (1 or 0)
  m: number;               // Manual mode (1 or 0)
  c: number;               // Auto cycle time (seconds)
  cd: number;              // Countdown seconds remaining
  reps?: number;           // Target repeat cycles per effect (1..50, default 3)
  cur_rep?: number;        // Current cycle number being executed (1..reps)
  cyc_mode?: number;       // Cycle mode: 0 = By Repeats, 1 = By Time
  temp: number;            // CPU temperature in Celsius
  heap: number;            // Free heap memory in bytes
  uptime: number;          // System uptime in seconds
  mqtt?: {
    connected: number;
    broker: string;
    port: number;
  };
  wifi?: {
    sta_ok: number;
    sta_ssid: string;
    sta_ip: string;
    rssi: number;
    ap_ip: string;
    mdns: string;
  };
  custom_pat?: {
    name: string;
    count: number;
  };
  ch: number[];            // 16 channels: 1 = ON, 0 = OFF
}

export type ConnectionMode = "cloud" | "local";

export interface CustomPattern {
  name: string;
  speed: number;
  frames: number[]; // 16-bit integer bitmasks
}

