"use client";

import { useState, useEffect, useRef, useCallback } from "react";
import mqtt, { MqttClient } from "mqtt";
import { DeviceState, ConnectionMode, CustomPattern } from "../types";

export const EFFECT_NAMES = [
  "Left Chaser", "Right Chaser", "Ping Pong", "Center Out", "Edges In", "Zip + Unzip",
  "Dual Snake", "Binary Counter", "Sparkle / Stardust", "Twinkle Sky", "Wave Fill",
  "Heartbeat Pulse", "Cylon Scanner", "Alternating Odd/Ev", "Falling Dominos", "Theater Marquee",
  "Raindrop Ripple", "Fireworks Burst", "Knight Rider", "Meteor Shower", "Double Bounce",
  "Cross Over", "Strobe Burst", "Stack Build", "Reverse Stack", "Quad Runner",
  "In-Out Breathe", "Segment Stride", "Police Strobe", "Curtain Reveal", "Cyber Glitch",
  "Ripple Reflect", "Marching Pairs", "Converge & Cross", "4-Phase Wave", "Harbor Radar",
  "Fill & Drain", "Random Walk", "DNA Helix", "All Breather", "Dynamic Custom Pattern"
];

const DEFAULT_STATE: DeviceState = {
  device_id: "relay16_default",
  e: 0,
  ename: "Left Chaser",
  s: 120,
  a: 0,
  sh: 0,
  r: 1,
  m: 0,
  c: 8,
  cd: 0,
  reps: 3,
  cur_rep: 1,
  cyc_mode: 0,
  temp: 42,
  heap: 250000,
  uptime: 0,
  ch: Array(16).fill(0),
  wifi: {
    sta_ok: 0,
    sta_ssid: "None",
    sta_ip: "192.168.4.1",
    rssi: -60,
    ap_ip: "192.168.4.1",
    mdns: "http://relay16.local",
  },
  mqtt: {
    connected: 0,
    broker: "broker.hivemq.com",
    port: 1883,
  },
  custom_pat: {
    name: "None",
    count: 0,
  },
};

export function sanitizeBrokerWsUrl(raw: string): string {
  let u = (raw || "").trim();
  if (!u || u.includes("emqx.io")) {
    return "wss://broker.hivemq.com:8884/mqtt";
  }
  // If user entered HiveMQ host with port 1883 or bare host:
  if (u.includes("broker.hivemq.com")) {
    return "wss://broker.hivemq.com:8884/mqtt";
  }
  // If user typed mqtt:// or tcp://, convert to wss:// or ws://
  if (u.startsWith("tcp://") || u.startsWith("mqtt://")) {
    u = u.replace(/^tcp:\/\//, "ws://").replace(/^mqtt:\/\//, "ws://");
  }
  // If it ends with :1883 (TCP port), convert to WebSocket port
  if (u.endsWith(":1883")) {
    return "wss://broker.hivemq.com:8884/mqtt";
  }
  // If missing ws/wss scheme, prepend wss://
  if (!u.startsWith("ws://") && !u.startsWith("wss://")) {
    u = `wss://${u}`;
  }
  return u;
}

export function useRelayController() {
  const [state, setState] = useState<DeviceState>(DEFAULT_STATE);
  const [connectionMode, setConnectionMode] = useState<ConnectionMode>("cloud");
  const [deviceId, setDeviceId] = useState<string>("relay16_default");
  const [brokerWsUrl, setBrokerWsUrl] = useState<string>("wss://broker.hivemq.com:8884/mqtt");
  const [localIp, setLocalIp] = useState<string>("relay16.local");
  const [isConnected, setIsConnected] = useState<boolean>(false);
  const [lastRxTime, setLastRxTime] = useState<number>(0);

  const clientRef = useRef<MqttClient | null>(null);

  // Load saved preferences from localStorage on mount
  useEffect(() => {
    try {
      const savedDevId = localStorage.getItem("relay_device_id");
      if (savedDevId) setDeviceId(savedDevId);

      const savedMode = localStorage.getItem("relay_mode") as ConnectionMode;
      if (savedMode) setConnectionMode(savedMode);

      const savedIp = localStorage.getItem("relay_local_ip");
      if (savedIp) setLocalIp(savedIp);

      const savedBroker = localStorage.getItem("relay_broker_ws");
      const cleanBroker = sanitizeBrokerWsUrl(savedBroker || "wss://broker.hivemq.com:8884/mqtt");
      setBrokerWsUrl(cleanBroker);
      localStorage.setItem("relay_broker_ws", cleanBroker);
    } catch {
      // Ignore in SSR
    }
  }, []);

  // MQTT Connection Logic (Cloud Mode - Zero Port Forwarding)
  useEffect(() => {
    if (connectionMode !== "cloud") {
      if (clientRef.current) {
        clientRef.current.end(true);
        clientRef.current = null;
      }
      return;
    }

    const clientId = `web_panel_${Math.random().toString(16).substring(2, 10)}`;
    const client = mqtt.connect(brokerWsUrl, {
      clientId,
      clean: true,
      connectTimeout: 8000,
      reconnectPeriod: 4000,
    });

    clientRef.current = client;

    client.on("connect", () => {
      setIsConnected(true);
      // Subscribe to state topic
      const topic = `${deviceId}/state`;
      client.subscribe(topic, (err) => {
        if (!err) {
          // Request initial state
          client.publish(`${deviceId}/cmd`, JSON.stringify({ cmd: "request_state" }));
        }
      });
    });

    client.on("message", (topic, message) => {
      try {
        const parsed = JSON.parse(message.toString());
        if (parsed && Array.isArray(parsed.ch)) {
          setState((prev) => ({
            ...prev,
            ...parsed,
          }));
          setLastRxTime(Date.now());
        }
      } catch (e) {
        console.error("MQTT JSON parse error:", e);
      }
    });

    client.on("close", () => setIsConnected(false));
    client.on("error", () => setIsConnected(false));

    return () => {
      client.end(true);
      clientRef.current = null;
    };
  }, [connectionMode, deviceId, brokerWsUrl]);

  // Local Polling Mode (Local LAN Mode)
  useEffect(() => {
    if (connectionMode !== "local") return;

    let active = true;
    const pollLocal = async () => {
      try {
        const url = `http://${localIp}/api/state`;
        const res = await fetch(url, { signal: AbortSignal.timeout(2500) });
        if (res.ok) {
          const d = await res.json();
          if (active) {
            setState(d);
            setIsConnected(true);
            setLastRxTime(Date.now());
          }
        } else {
          if (active) setIsConnected(false);
        }
      } catch {
        if (active) setIsConnected(false);
      }
    };

    pollLocal();
    const interval = setInterval(pollLocal, 1000);
    return () => {
      active = false;
      clearInterval(interval);
    };
  }, [connectionMode, localIp]);

  // Send Command Helper
  const sendCommand = useCallback(
    (payload: Record<string, unknown>) => {
      if (connectionMode === "cloud") {
        if (clientRef.current && clientRef.current.connected) {
          clientRef.current.publish(`${deviceId}/cmd`, JSON.stringify(payload));
        }
      } else {
        // Local mode HTTP dispatch
        const cmd = payload.cmd as string;
        let url = `http://${localIp}/api/cmd?a=`;
        if (cmd === "relay") {
          url = `http://${localIp}/api/relay?ch=${payload.ch}&v=${payload.v}`;
        } else if (cmd === "effect") {
          url = `http://${localIp}/api/effect?id=${payload.id}`;
        } else if (cmd === "speed") {
          url = `http://${localIp}/api/speed?v=${payload.v}`;
        } else if (cmd === "cycle") {
          url = `http://${localIp}/api/cycle?v=${payload.v}`;
        } else if (cmd === "action") {
          url = `http://${localIp}/api/cmd?a=${payload.a}`;
        } else if (cmd === "repeats") {
          url = `http://${localIp}/api/repeats?v=${payload.v}`;
        } else if (cmd === "cyc_mode") {
          url = `http://${localIp}/api/cycmode?v=${payload.v}`;
        }
        fetch(url, { mode: "no-cors" }).catch(() => {});
      }
    },
    [connectionMode, deviceId, localIp]
  );

  // Relay actions
  const toggleRelay = useCallback(
    (ch: number) => {
      const current = state.ch[ch] === 1;
      const nextVal = current ? 0 : 1;
      // Optimistic update
      setState((prev) => {
        const nextCh = [...prev.ch];
        nextCh[ch] = nextVal;
        return { ...prev, ch: nextCh, m: 1, r: 0 };
      });
      sendCommand({ cmd: "relay", ch, v: nextVal });
    },
    [state.ch, sendCommand]
  );

  const setEffect = useCallback(
    (id: number) => {
      setState((prev) => ({
        ...prev,
        e: id,
        ename: EFFECT_NAMES[id] || `Effect ${id}`,
        m: 0,
        r: 1,
      }));
      sendCommand({ cmd: "effect", id });
    },
    [sendCommand]
  );

  const setSpeed = useCallback(
    (speedMs: number) => {
      setState((prev) => ({ ...prev, s: speedMs }));
      sendCommand({ cmd: "speed", v: speedMs });
    },
    [sendCommand]
  );

  const setCycle = useCallback(
    (seconds: number) => {
      setState((prev) => ({ ...prev, c: seconds }));
      sendCommand({ cmd: "cycle", v: seconds * 1000 });
    },
    [sendCommand]
  );

  const setRepeats = useCallback(
    (repeats: number) => {
      setState((prev) => ({ ...prev, reps: repeats, cyc_mode: 0 }));
      if (connectionMode === "cloud") {
        sendCommand({ cmd: "repeats", v: repeats });
      } else {
        fetch(`http://${localIp}/api/repeats?v=${repeats}`).catch(() => {});
      }
    },
    [connectionMode, sendCommand, localIp]
  );

  const setCycMode = useCallback(
    (mode: number) => {
      setState((prev) => ({ ...prev, cyc_mode: mode }));
      if (connectionMode === "cloud") {
        sendCommand({ cmd: "cyc_mode", v: mode });
      } else {
        fetch(`http://${localIp}/api/cycmode?v=${mode}`).catch(() => {});
      }
    },
    [connectionMode, sendCommand, localIp]
  );

  const triggerAction = useCallback(
    (action: string) => {
      if (action === "all_on") {
        setState((prev) => ({ ...prev, ch: Array(16).fill(1), m: 1, r: 0 }));
      } else if (action === "all_off") {
        setState((prev) => ({ ...prev, ch: Array(16).fill(0), m: 1, r: 0 }));
      } else if (action === "toggle_run") {
        setState((prev) => ({ ...prev, r: prev.r ? 0 : 1 }));
      } else if (action === "toggle_auto") {
        setState((prev) => ({ ...prev, a: prev.a ? 0 : 1 }));
      } else if (action === "toggle_shuffle") {
        setState((prev) => ({ ...prev, sh: prev.sh ? 0 : 1 }));
      } else if (action === "toggle_manual") {
        setState((prev) => ({ ...prev, m: prev.m ? 0 : 1 }));
      }
      sendCommand({ cmd: "action", a: action });
    },
    [sendCommand]
  );

  // Push Dynamic Custom Pattern without flashing
  const uploadCustomPattern = useCallback(
    (pattern: CustomPattern) => {
      const payload = {
        cmd: "load_pattern",
        name: pattern.name,
        speed: pattern.speed,
        frames: pattern.frames,
      };

      if (connectionMode === "cloud") {
        sendCommand(payload);
      } else {
        fetch(`http://${localIp}/api/pattern/load`, {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(payload),
        }).catch(() => {});
      }

      setState((prev) => ({
        ...prev,
        e: 40,
        ename: pattern.name,
        s: pattern.speed,
        m: 0,
        r: 1,
        custom_pat: {
          name: pattern.name,
          count: pattern.frames.length,
        },
      }));
    },
    [connectionMode, sendCommand, localIp]
  );

  // Live Stream a Single 16-bit Frame
  const streamFrame = useCallback(
    (mask: number) => {
      sendCommand({ cmd: "stream_frame", mask });
    },
    [sendCommand]
  );

  const updateConfig = (newDevId: string, newMode: ConnectionMode, newIp: string, newBroker: string) => {
    const cleanBroker = sanitizeBrokerWsUrl(newBroker);
    setDeviceId(newDevId);
    setConnectionMode(newMode);
    setLocalIp(newIp);
    setBrokerWsUrl(cleanBroker);

    localStorage.setItem("relay_device_id", newDevId);
    localStorage.setItem("relay_mode", newMode);
    localStorage.setItem("relay_local_ip", newIp);
    localStorage.setItem("relay_broker_ws", cleanBroker);
  };

  return {
    state,
    isConnected,
    connectionMode,
    deviceId,
    localIp,
    brokerWsUrl,
    lastRxTime,
    toggleRelay,
    setEffect,
    setSpeed,
    setCycle,
    setRepeats,
    setCycMode,
    triggerAction,
    uploadCustomPattern,
    streamFrame,
    updateConfig,
  };
}

