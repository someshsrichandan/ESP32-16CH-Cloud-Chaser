"use client";

import React from "react";
import { Power, Radio } from "lucide-react";

interface RelayVisualizerProps {
  channels: number[];
  onToggleRelay: (channel: number) => void;
  onAllOn: () => void;
  onAllOff: () => void;
  onInvert: () => void;
  onOdd: () => void;
  onEven: () => void;
}

export const RelayVisualizer: React.FC<RelayVisualizerProps> = ({
  channels,
  onToggleRelay,
  onAllOn,
  onAllOff,
  onInvert,
  onOdd,
  onEven,
}) => {
  const activeCount = channels.filter((c) => c === 1).length;

  return (
    <div className="bg-slate-900/60 border border-slate-800/80 rounded-2xl p-4 md:p-6 shadow-xl backdrop-blur-md">
      {/* Title & Quick Actions */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-5">
        <div className="flex items-center gap-2.5">
          <Radio className="w-5 h-5 text-cyan-400" />
          <h2 className="text-base font-bold text-white tracking-wide uppercase">
            16-Channel Relays
          </h2>
          <span className="text-xs font-mono font-semibold px-2 py-0.5 rounded-full bg-cyan-950/60 text-cyan-300 border border-cyan-800/50">
            {activeCount} Active
          </span>
        </div>

        {/* Quick Batch Controls */}
        <div className="flex flex-wrap items-center gap-1.5">
          <button
            onClick={onAllOn}
            className="px-3 py-1.5 text-xs font-bold rounded-lg bg-emerald-600/20 text-emerald-300 border border-emerald-600/40 hover:bg-emerald-600/30 transition-all active:scale-95"
          >
            All ON
          </button>
          <button
            onClick={onAllOff}
            className="px-3 py-1.5 text-xs font-bold rounded-lg bg-red-600/20 text-red-300 border border-red-600/40 hover:bg-red-600/30 transition-all active:scale-95"
          >
            All OFF
          </button>
          <button
            onClick={onInvert}
            className="px-3 py-1.5 text-xs font-bold rounded-lg bg-purple-600/20 text-purple-300 border border-purple-600/40 hover:bg-purple-600/30 transition-all active:scale-95"
          >
            Invert
          </button>
          <button
            onClick={onOdd}
            className="px-2.5 py-1.5 text-xs font-bold rounded-lg bg-slate-800 text-slate-300 border border-slate-700 hover:bg-slate-700 transition-all active:scale-95"
          >
            Odd
          </button>
          <button
            onClick={onEven}
            className="px-2.5 py-1.5 text-xs font-bold rounded-lg bg-slate-800 text-slate-300 border border-slate-700 hover:bg-slate-700 transition-all active:scale-95"
          >
            Even
          </button>
        </div>
      </div>

      {/* 16 Channel Grid */}
      <div className="grid grid-cols-4 sm:grid-cols-8 gap-2.5 md:gap-3">
        {channels.map((state, idx) => {
          const isOn = state === 1;
          return (
            <button
              key={idx}
              onClick={() => onToggleRelay(idx)}
              className={`relative flex flex-col items-center justify-between p-3 rounded-xl border transition-all duration-150 user-select-none group ${
                isOn
                  ? "bg-gradient-to-b from-emerald-600/30 to-emerald-950/60 border-emerald-500/80 shadow-lg shadow-emerald-500/30 text-white scale-[1.02]"
                  : "bg-slate-950/70 border-slate-800/80 text-slate-400 hover:border-slate-700 hover:bg-slate-900/60"
              } active:scale-90`}
            >
              {/* Channel Label */}
              <div className="w-full flex items-center justify-between">
                <span className="text-[11px] font-black text-slate-400 group-hover:text-slate-200">
                  CH{idx + 1}
                </span>
                <span
                  className={`w-2 h-2 rounded-full ${
                    isOn ? "bg-emerald-400 shadow-sm shadow-emerald-400 animate-pulse" : "bg-slate-700"
                  }`}
                />
              </div>

              {/* Status Indicator Icon */}
              <div className="my-2">
                <Power
                  className={`w-6 h-6 transition-all ${
                    isOn
                      ? "text-emerald-400 drop-shadow-[0_0_8px_rgba(52,211,153,0.8)] scale-110"
                      : "text-slate-600 group-hover:text-slate-500"
                  }`}
                />
              </div>

              {/* ON/OFF Text */}
              <span
                className={`text-[10px] font-black tracking-wider uppercase ${
                  isOn ? "text-emerald-300" : "text-slate-600"
                }`}
              >
                {isOn ? "CLOSED" : "OPEN"}
              </span>
            </button>
          );
        })}
      </div>
    </div>
  );
};

