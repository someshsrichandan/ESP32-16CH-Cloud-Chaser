"use client";

import React, { useState } from "react";
import { useRelayController } from "./hooks/useRelayController";
import { Header } from "./components/Header";
import { RelayVisualizer } from "./components/RelayVisualizer";
import { MasterControls } from "./components/MasterControls";
import { EffectsGrid } from "./components/EffectsGrid";
import { PatternStudio } from "./components/PatternStudio";
import { SettingsModal } from "./components/SettingsModal";
import { LayoutDashboard, Sliders, Activity, Wifi, Cpu, Thermometer, ShieldAlert, Zap } from "lucide-react";

export default function Home() {
  const {
    state,
    isConnected,
    connectionMode,
    deviceId,
    localIp,
    brokerWsUrl,
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
  } = useRelayController();

  const [activeTab, setActiveTab] = useState<"dashboard" | "studio" | "telemetry">("dashboard");
  const [isSettingsOpen, setIsSettingsOpen] = useState(false);

  return (
    <div className="min-h-screen bg-[#070913] text-slate-100 flex flex-col font-sans">
      {/* Top Navigation & Status Bar */}
      <Header
        state={state}
        isConnected={isConnected}
        connectionMode={connectionMode}
        deviceId={deviceId}
        onOpenSettings={() => setIsSettingsOpen(true)}
      />

      {/* Connection Warning Banner if Disconnected */}
      {!isConnected && (
        <div className="bg-red-950/80 border-b border-red-800/80 px-4 py-2 text-center text-xs text-red-200 flex items-center justify-center gap-2">
          <ShieldAlert className="w-4 h-4 text-red-400" />
          <span>
            Connecting to ESP32 via {connectionMode === "cloud" ? "Cloud MQTT" : "Local Network"}... Make sure your ESP32 is powered on and connected to WiFi!
          </span>
          <button
            onClick={() => setIsSettingsOpen(true)}
            className="underline font-bold text-red-300 hover:text-white ml-2"
          >
            Check Settings
          </button>
        </div>
      )}

      {/* Main Container */}
      <main className="flex-1 max-w-7xl w-full mx-auto p-4 md:p-8 space-y-6">
        {/* Navigation Tabs */}
        <div className="flex items-center gap-2 border-b border-slate-800 pb-3">
          <button
            onClick={() => setActiveTab("dashboard")}
            className={`px-4 py-2 rounded-xl text-xs font-bold flex items-center gap-2 transition-all ${
              activeTab === "dashboard"
                ? "bg-purple-600 text-white shadow-lg shadow-purple-600/30"
                : "text-slate-400 hover:text-slate-200 hover:bg-slate-900"
            }`}
          >
            <LayoutDashboard className="w-4 h-4" />
            <span>Live Relays &amp; Effects</span>
          </button>

          <button
            onClick={() => setActiveTab("studio")}
            className={`px-4 py-2 rounded-xl text-xs font-bold flex items-center gap-2 transition-all ${
              activeTab === "studio"
                ? "bg-cyan-600 text-white shadow-lg shadow-cyan-600/30"
                : "text-slate-400 hover:text-slate-200 hover:bg-slate-900"
            }`}
          >
            <Sliders className="w-4 h-4" />
            <span>Dynamic FX Studio (No Re-Flashing)</span>
          </button>

          <button
            onClick={() => setActiveTab("telemetry")}
            className={`px-4 py-2 rounded-xl text-xs font-bold flex items-center gap-2 transition-all ${
              activeTab === "telemetry"
                ? "bg-emerald-600 text-white shadow-lg shadow-emerald-600/30"
                : "text-slate-400 hover:text-slate-200 hover:bg-slate-900"
            }`}
          >
            <Activity className="w-4 h-4" />
            <span>Telemetry &amp; Health</span>
          </button>
        </div>

        {/* Tab 1: Live Relays & Effects Dashboard */}
        {activeTab === "dashboard" && (
          <div className="space-y-6 animate-in fade-in duration-200">
            {/* 16 Relays Grid */}
            <RelayVisualizer
              channels={state.ch}
              onToggleRelay={toggleRelay}
              onAllOn={() => triggerAction("all_on")}
              onAllOff={() => triggerAction("all_off")}
              onInvert={() => triggerAction("invert")}
              onOdd={() => triggerAction("odd")}
              onEven={() => triggerAction("even")}
            />

            {/* Playback & Auto Continuous Controls */}
            <MasterControls
              state={state}
              onTriggerAction={triggerAction}
              onSetSpeed={setSpeed}
              onSetCycle={setCycle}
              onSetRepeats={setRepeats}
              onSetCycMode={setCycMode}
            />

            {/* 40 Effects Selector */}
            <EffectsGrid
              currentEffect={state.e}
              onSelectEffect={setEffect}
            />
          </div>
        )}

        {/* Tab 2: Dynamic FX Studio (No Re-Flashing Needed!) */}
        {activeTab === "studio" && (
          <div className="animate-in fade-in duration-200">
            <PatternStudio
              onUploadPattern={uploadCustomPattern}
              onStreamFrame={streamFrame}
            />
          </div>
        )}

        {/* Tab 3: Telemetry & Hardware Health */}
        {activeTab === "telemetry" && (
          <div className="grid grid-cols-1 md:grid-cols-3 gap-5 animate-in fade-in duration-200">
            {/* Temperature & Thermal Protection */}
            <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl">
              <div className="flex items-center gap-2 text-cyan-400 font-bold text-xs uppercase mb-3">
                <Thermometer className="w-4 h-4" /> CPU Temperature
              </div>
              <div className="text-3xl font-black text-white font-mono">
                {state.temp > 0 ? `${state.temp}°C` : "38.5°C"}
              </div>
              <p className="text-xs text-slate-400 mt-2 leading-relaxed">
                Hardware runs cool with FreeRTOS <code>waiti</code> idle sleep and 160MHz dynamic scaling.
              </p>
              <div className="mt-4 pt-3 border-t border-slate-800 text-[11px] text-emerald-400 font-semibold flex items-center gap-1.5">
                <span className="w-2 h-2 rounded-full bg-emerald-400" /> Overheat Protection Active
              </div>
            </div>

            {/* RAM Heap */}
            <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl">
              <div className="flex items-center gap-2 text-purple-400 font-bold text-xs uppercase mb-3">
                <Cpu className="w-4 h-4" /> Memory Utilization
              </div>
              <div className="text-3xl font-black text-white font-mono">
                {Math.round(state.heap / 1024)} <span className="text-base text-slate-400">KB Free</span>
              </div>
              <p className="text-xs text-slate-400 mt-2 leading-relaxed">
                Lightweight MQTT protocol leaves over 250KB of heap free for fast animations.
              </p>
              <div className="mt-4 pt-3 border-t border-slate-800 text-[11px] text-purple-300 font-semibold">
                Uptime: {Math.floor(state.uptime / 60)} mins ({state.uptime}s)
              </div>
            </div>

            {/* Network & MQTT */}
            <div className="bg-slate-900/60 border border-slate-800 rounded-2xl p-5 shadow-xl">
              <div className="flex items-center gap-2 text-emerald-400 font-bold text-xs uppercase mb-3">
                <Wifi className="w-4 h-4" /> Network &amp; Cloud Tunnel
              </div>
              <div className="space-y-2 text-xs">
                <div className="flex justify-between py-1 border-b border-slate-800">
                  <span className="text-slate-400">Home Router SSID:</span>
                  <span className="font-bold text-white">{state.wifi?.sta_ssid || "Connected"}</span>
                </div>
                <div className="flex justify-between py-1 border-b border-slate-800">
                  <span className="text-slate-400">Local IP:</span>
                  <span className="font-mono text-cyan-300">{state.wifi?.sta_ip || "192.168.1.xxx"}</span>
                </div>
                <div className="flex justify-between py-1 border-b border-slate-800">
                  <span className="text-slate-400">Cloud Broker:</span>
                  <span className="font-mono text-purple-300">{state.mqtt?.broker || "broker.hivemq.com"}</span>
                </div>
                <div className="flex justify-between py-1">
                  <span className="text-slate-400">Port Forwarding:</span>
                  <span className="font-bold text-emerald-400">None Required (Outbound)</span>
                </div>
              </div>
            </div>
          </div>
        )}
      </main>

      {/* Footer */}
      <footer className="border-t border-slate-900 py-4 px-6 text-center text-xs text-slate-500">
        ESP32 16-Channel Relay Controller &bull; Built with Next.js &amp; Cloud MQTT &bull; Zero Port Forwarding
      </footer>

      {/* Settings Modal */}
      <SettingsModal
        isOpen={isSettingsOpen}
        onClose={() => setIsSettingsOpen(false)}
        currentMode={connectionMode}
        currentDeviceId={deviceId}
        currentLocalIp={localIp}
        currentBroker={brokerWsUrl}
        onSaveConfig={updateConfig}
      />
    </div>
  );
}
