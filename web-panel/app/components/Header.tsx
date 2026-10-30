"use client";

import React, { useState } from "react";
import { DeviceState, ConnectionMode } from "../types";
import { Wifi, Cpu, Thermometer, Settings, Cloud, Server, Zap, Copy, Check } from "lucide-react";

interface HeaderProps {
  state: DeviceState;
  isConnected: boolean;
  connectionMode: ConnectionMode;
  deviceId: string;
  onOpenSettings: () => void;
}

export const Header: React.FC<HeaderProps> = ({
  state,
  isConnected,
  connectionMode,
  deviceId,
  onOpenSettings,
}) => {
  const isWarm = state.temp > 50;
  const [copied, setCopied] = useState(false);

  const handleCopy = () => {
    if (navigator.clipboard) {
      navigator.clipboard.writeText(deviceId);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
    }
  };

  return (
    <header className="relative bg-gradient-to-r from-[#12072b] via-[#090b1c] to-[#04152d] border-b border-purple-900/30 px-4 py-4 md:px-8 shadow-2xl">
      <div className="max-w-7xl mx-auto flex flex-col md:flex-row items-center justify-between gap-4">
        {/* Title & Brand */}
        <div className="flex items-center gap-3">
          <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-purple-600 to-cyan-500 flex items-center justify-center shadow-lg shadow-purple-500/20">
            <Zap className="w-6 h-6 text-white" />
          </div>
          <div>
            <h1 className="text-xl md:text-2xl font-black bg-gradient-to-r from-purple-400 via-cyan-400 to-pink-400 bg-clip-text text-transparent tracking-wide">
              ESP32 × 16-CH RELAY PRO
            </h1>
            <p className="text-xs text-slate-400 flex flex-wrap items-center gap-2 font-medium">
              <span>HiveMQ Cloud IoT</span>
              <span className="w-1 h-1 rounded-full bg-slate-600"></span>
              <span className="text-cyan-400">Zero Port Forwarding</span>
              <span className="w-1 h-1 rounded-full bg-slate-600"></span>
              <button
                onClick={handleCopy}
                className="font-mono text-purple-300 hover:text-white flex items-center gap-1 bg-purple-950/40 px-1.5 py-0.5 rounded border border-purple-800/40 hover:border-purple-600 transition-all"
                title="Click to copy Device ID"
              >
                <span>{deviceId}</span>
                {copied ? <Check className="w-3 h-3 text-emerald-400" /> : <Copy className="w-3 h-3 text-slate-400" />}
              </button>
            </p>
          </div>
        </div>

        {/* Telemetry & Status Badges */}
        <div className="flex flex-wrap items-center justify-center gap-2">
          {/* Connection Badge */}
          <div
            className={`px-3 py-1.5 rounded-full text-xs font-semibold flex items-center gap-2 border transition-all ${
              isConnected
                ? "bg-emerald-950/40 border-emerald-500/40 text-emerald-300 shadow-sm shadow-emerald-500/20"
                : "bg-red-950/40 border-red-500/40 text-red-300 animate-pulse"
            }`}
          >
            <span
              className={`w-2.5 h-2.5 rounded-full ${
                isConnected ? "bg-emerald-400 shadow-sm shadow-emerald-400" : "bg-red-400"
              }`}
            />
            {isConnected ? (
              connectionMode === "cloud" ? (
                <span className="flex items-center gap-1.5">
                  <Cloud className="w-3.5 h-3.5 text-cyan-400" /> Cloud Active
                </span>
              ) : (
                <span className="flex items-center gap-1.5">
                  <Server className="w-3.5 h-3.5 text-emerald-400" /> Local LAN
                </span>
              )
            ) : (
              "Disconnected"
            )}
          </div>

          {/* Temperature Badge */}
          <div
            className={`px-3 py-1.5 rounded-full text-xs font-semibold flex items-center gap-1.5 border ${
              isWarm
                ? "bg-amber-950/40 border-amber-500/40 text-amber-300"
                : "bg-slate-900/60 border-slate-700/60 text-slate-300"
            }`}
          >
            <Thermometer className="w-3.5 h-3.5 text-cyan-400" />
            <span>{state.temp > 0 ? `${state.temp}°C` : "Cool"}</span>
          </div>

          {/* Heap Memory Badge */}
          <div className="hidden sm:flex px-3 py-1.5 rounded-full text-xs font-semibold items-center gap-1.5 bg-slate-900/60 border border-slate-700/60 text-slate-300">
            <Cpu className="w-3.5 h-3.5 text-purple-400" />
            <span>{Math.round(state.heap / 1024)} KB Free</span>
          </div>

          {/* WiFi RSSI */}
          {state.wifi?.sta_ok === 1 && (
            <div className="hidden md:flex px-3 py-1.5 rounded-full text-xs font-semibold items-center gap-1.5 bg-slate-900/60 border border-slate-700/60 text-slate-300">
              <Wifi className="w-3.5 h-3.5 text-emerald-400" />
              <span>{state.wifi.sta_ssid} ({state.wifi.rssi} dBm)</span>
            </div>
          )}

          {/* Settings Button */}
          <button
            onClick={onOpenSettings}
            className="p-2 rounded-xl bg-purple-950/40 border border-purple-700/50 hover:bg-purple-900/60 text-purple-300 hover:text-white transition-all shadow-md hover:scale-105 active:scale-95"
            title="Configure Connection & WiFi"
          >
            <Settings className="w-4 h-4" />
          </button>
        </div>
      </div>
    </header>
  );
};

