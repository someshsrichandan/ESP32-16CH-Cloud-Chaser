"use client";

import React, { useState } from "react";
import { ConnectionMode } from "../types";
import { X, Cloud, Server, HelpCircle, Save, ExternalLink } from "lucide-react";

interface SettingsModalProps {
  isOpen: boolean;
  onClose: () => void;
  currentMode: ConnectionMode;
  currentDeviceId: string;
  currentLocalIp: string;
  currentBroker: string;
  onSaveConfig: (devId: string, mode: ConnectionMode, ip: string, broker: string) => void;
}

export const SettingsModal: React.FC<SettingsModalProps> = ({
  isOpen,
  onClose,
  currentMode,
  currentDeviceId,
  currentLocalIp,
  currentBroker,
  onSaveConfig,
}) => {
  const [mode, setMode] = useState<ConnectionMode>(currentMode);
  const [devId, setDevId] = useState(currentDeviceId);
  const [localIp, setLocalIp] = useState(currentLocalIp);
  const [broker, setBroker] = useState(currentBroker);

  if (!isOpen) return null;

  const handleSave = () => {
    onSaveConfig(devId.trim(), mode, localIp.trim(), broker.trim());
    onClose();
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/80 backdrop-blur-sm animate-in fade-in duration-150">
      <div className="bg-[#0f1225] border border-purple-900/50 rounded-2xl w-full max-w-lg shadow-2xl p-6 text-slate-200 relative max-h-[90vh] overflow-y-auto">
        {/* Close Button */}
        <button
          onClick={onClose}
          className="absolute top-4 right-4 p-1.5 text-slate-400 hover:text-white rounded-lg hover:bg-slate-800 transition-all"
        >
          <X className="w-5 h-5" />
        </button>

        {/* Modal Header */}
        <h2 className="text-lg font-bold text-white mb-1 flex items-center gap-2">
          <span>Connection &amp; Remote Access Settings</span>
        </h2>
        <p className="text-xs text-slate-400 mb-5">
          Configure how this Next.js web panel connects to your ESP32 controller.
        </p>

        {/* Mode Selector */}
        <div className="mb-5">
          <label className="text-xs font-bold text-slate-300 block mb-2">Connection Mode</label>
          <div className="grid grid-cols-2 gap-3">
            <button
              type="button"
              onClick={() => setMode("cloud")}
              className={`p-3 rounded-xl border text-left flex flex-col justify-between transition-all ${
                mode === "cloud"
                  ? "bg-purple-950/60 border-purple-500 text-white shadow-lg shadow-purple-600/20"
                  : "bg-slate-950 border-slate-800 text-slate-400 hover:border-slate-700"
              }`}
            >
              <div className="flex items-center gap-2 font-bold text-xs mb-1">
                <Cloud className="w-4 h-4 text-cyan-400" /> Cloud MQTT
              </div>
              <span className="text-[10px] text-slate-400 leading-tight">
                Worldwide access from anywhere (4G/5G). Zero port forwarding needed!
              </span>
            </button>

            <button
              type="button"
              onClick={() => setMode("local")}
              className={`p-3 rounded-xl border text-left flex flex-col justify-between transition-all ${
                mode === "local"
                  ? "bg-purple-950/60 border-purple-500 text-white shadow-lg shadow-purple-600/20"
                  : "bg-slate-950 border-slate-800 text-slate-400 hover:border-slate-700"
              }`}
            >
              <div className="flex items-center gap-2 font-bold text-xs mb-1">
                <Server className="w-4 h-4 text-emerald-400" /> Local Network
              </div>
              <span className="text-[10px] text-slate-400 leading-tight">
                Direct connection over home WiFi (http://relay16.local).
              </span>
            </button>
          </div>
        </div>

        {/* Cloud Mode Settings */}
        {mode === "cloud" ? (
          <div className="space-y-3.5 mb-5 p-4 bg-slate-950/70 border border-slate-800 rounded-xl">
            <div>
              <label className="text-xs font-bold text-slate-300 block mb-1">
                ESP32 Device ID
              </label>
              <input
                type="text"
                value={devId}
                onChange={(e) => setDevId(e.target.value)}
                placeholder="e.g. relay16_xxxxxx"
                className="w-full bg-slate-900 border border-slate-700 rounded-xl px-3 py-2 text-xs text-white focus:outline-none focus:border-cyan-400 font-mono"
              />
              <span className="text-[10px] text-slate-500 mt-1 block">
                Found on serial monitor or local web interface (e.g. <code>relay16_a1b2c3</code>).
              </span>
            </div>

            <div>
              <label className="text-xs font-bold text-slate-300 block mb-1">
                MQTT WebSocket Broker
              </label>
              <input
                type="text"
                value={broker}
                onChange={(e) => setBroker(e.target.value)}
                placeholder="wss://broker.emqx.io:8084/mqtt"
                className="w-full bg-slate-900 border border-slate-700 rounded-xl px-3 py-2 text-xs text-white focus:outline-none focus:border-cyan-400 font-mono"
              />
              <span className="text-[10px] text-slate-500 mt-1 block">
                Default: <code>wss://broker.emqx.io:8084/mqtt</code> (free public secure broker).
              </span>
            </div>

            <div className="flex items-start gap-2 p-2.5 bg-cyan-950/20 border border-cyan-800/30 rounded-lg text-[11px] text-cyan-200/90 leading-relaxed">
              <HelpCircle className="w-4 h-4 text-cyan-400 flex-shrink-0 mt-0.5" />
              <span>
                <strong>Why Zero Port Forwarding?</strong> The ESP32 maintains an outbound TLS/TCP socket to the MQTT cloud. Your home router automatically allows outbound traffic, so you never have to open ports or expose your IP!
              </span>
            </div>
          </div>
        ) : (
          /* Local Mode Settings */
          <div className="space-y-3.5 mb-5 p-4 bg-slate-950/70 border border-slate-800 rounded-xl">
            <div>
              <label className="text-xs font-bold text-slate-300 block mb-1">
                ESP32 Local Address or IP
              </label>
              <input
                type="text"
                value={localIp}
                onChange={(e) => setLocalIp(e.target.value)}
                placeholder="relay16.local or 192.168.1.150"
                className="w-full bg-slate-900 border border-slate-700 rounded-xl px-3 py-2 text-xs text-white focus:outline-none focus:border-cyan-400 font-mono"
              />
              <span className="text-[10px] text-slate-500 mt-1 block">
                Default: <code>relay16.local</code> (or the IP assigned by your router).
              </span>
            </div>
          </div>
        )}

        {/* Web OTA Link */}
        <div className="mb-6 p-3 bg-slate-950/40 border border-slate-800 rounded-xl flex items-center justify-between text-xs">
          <div>
            <span className="font-bold text-slate-300 block">Web Firmware OTA</span>
            <span className="text-[11px] text-slate-500">Flash new .bin files without USB cable</span>
          </div>
          <a
            href={`http://${localIp}/update`}
            target="_blank"
            rel="noopener noreferrer"
            className="flex items-center gap-1 px-3 py-1.5 rounded-lg bg-slate-800 hover:bg-slate-700 text-purple-300 font-bold text-xs"
          >
            <span>Open OTA</span>
            <ExternalLink className="w-3.5 h-3.5" />
          </a>
        </div>

        {/* Action Buttons */}
        <div className="flex justify-end gap-2">
          <button
            onClick={onClose}
            className="px-4 py-2 rounded-xl text-xs font-bold text-slate-400 hover:text-white bg-slate-800 hover:bg-slate-700 transition-all"
          >
            Cancel
          </button>
          <button
            onClick={handleSave}
            className="px-5 py-2 rounded-xl text-xs font-bold text-white bg-gradient-to-r from-purple-600 to-cyan-600 hover:from-purple-500 hover:to-cyan-500 transition-all shadow-lg shadow-purple-600/30 flex items-center gap-1.5 active:scale-95"
          >
            <Save className="w-4 h-4" /> Save &amp; Connect
          </button>
        </div>
      </div>
    </div>
  );
};

