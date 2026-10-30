"use client";

import React from "react";
import { DeviceState } from "../types";
import { Play, Pause, Shuffle, RefreshCw, ChevronLeft, ChevronRight, Gauge, Timer } from "lucide-react";

interface MasterControlsProps {
  state: DeviceState;
  onTriggerAction: (action: string) => void;
  onSetSpeed: (speed: number) => void;
  onSetCycle: (seconds: number) => void;
}

export const MasterControls: React.FC<MasterControlsProps> = ({
  state,
  onTriggerAction,
  onSetSpeed,
  onSetCycle,
}) => {
  const isRunning = state.r === 1;
  const isAuto = state.a === 1;
  const isShuffle = state.sh === 1;
  const isManual = state.m === 1;

  // Auto cycle countdown percentage
  const cycleTotal = state.c > 0 ? state.c : 8;
  const cycleRemain = state.cd >= 0 ? state.cd : 0;
  const cyclePct = isAuto ? Math.max(0, Math.min(100, ((cycleTotal - cycleRemain) / cycleTotal) * 100)) : 0;

  return (
    <div className="bg-slate-900/60 border border-slate-800/80 rounded-2xl p-4 md:p-6 shadow-xl backdrop-blur-md">
      {/* Title & Mode Status */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-5">
        <div>
          <h2 className="text-base font-bold text-white tracking-wide uppercase flex items-center gap-2">
            <span>Play &amp; Continuous Controls</span>
          </h2>
          <p className="text-xs text-slate-400 mt-0.5">
            Current:{" "}
            <span className="font-bold text-purple-300">
              {state.ename || `Effect ${state.e}`}
            </span>{" "}
            {isManual && (
              <span className="ml-2 px-2 py-0.5 rounded text-[10px] font-bold bg-amber-500/20 text-amber-300 border border-amber-500/40">
                MANUAL OVERRIDE
              </span>
            )}
          </p>
        </div>

        {/* Action Buttons */}
        <div className="flex flex-wrap items-center gap-2">
          {/* Pause / Play */}
          <button
            onClick={() => onTriggerAction("toggle_run")}
            className={`px-4 py-2 rounded-xl text-xs font-bold flex items-center gap-2 transition-all shadow-md active:scale-95 ${
              isRunning
                ? "bg-blue-600/30 text-blue-200 border border-blue-500/50 hover:bg-blue-600/40"
                : "bg-emerald-600/30 text-emerald-200 border border-emerald-500/50 hover:bg-emerald-600/40 animate-pulse"
            }`}
          >
            {isRunning ? (
              <>
                <Pause className="w-3.5 h-3.5 fill-current" /> Pause
              </>
            ) : (
              <>
                <Play className="w-3.5 h-3.5 fill-current" /> Resume
              </>
            )}
          </button>

          {/* Auto Mode Toggle */}
          <button
            onClick={() => onTriggerAction("toggle_auto")}
            className={`px-3 py-2 rounded-xl text-xs font-bold flex items-center gap-1.5 transition-all shadow-md active:scale-95 ${
              isAuto
                ? "bg-purple-600 text-white border border-purple-400 shadow-purple-600/30"
                : "bg-slate-800 text-slate-300 border border-slate-700 hover:bg-slate-700"
            }`}
          >
            <RefreshCw className={`w-3.5 h-3.5 ${isAuto ? "animate-spin" : ""}`} />
            <span>Auto: {isAuto ? "ON" : "OFF"}</span>
          </button>

          {/* Shuffle Mode Toggle */}
          <button
            onClick={() => onTriggerAction("toggle_shuffle")}
            className={`px-3 py-2 rounded-xl text-xs font-bold flex items-center gap-1.5 transition-all shadow-md active:scale-95 ${
              isShuffle
                ? "bg-cyan-600 text-white border border-cyan-400 shadow-cyan-600/30"
                : "bg-slate-800 text-slate-300 border border-slate-700 hover:bg-slate-700"
            }`}
          >
            <Shuffle className="w-3.5 h-3.5" />
            <span>Shuffle: {isShuffle ? "ON" : "OFF"}</span>
          </button>

          {/* Prev / Next */}
          <div className="flex items-center gap-1 bg-slate-950 p-1 rounded-xl border border-slate-800">
            <button
              onClick={() => onTriggerAction("prev")}
              className="p-1.5 rounded-lg text-slate-300 hover:text-white hover:bg-slate-800 active:scale-90 transition-all"
              title="Previous Effect"
            >
              <ChevronLeft className="w-4 h-4" />
            </button>
            <button
              onClick={() => onTriggerAction("next")}
              className="p-1.5 rounded-lg text-slate-300 hover:text-white hover:bg-slate-800 active:scale-90 transition-all"
              title="Next Effect"
            >
              <ChevronRight className="w-4 h-4" />
            </button>
          </div>
        </div>
      </div>

      {/* Auto Mode Countdown Bar */}
      {isAuto && (
        <div className="mb-5">
          <div className="flex items-center justify-between text-xs text-slate-400 mb-1.5 font-medium">
            <span className="flex items-center gap-1.5">
              <RefreshCw className="w-3 h-3 text-purple-400 animate-spin" /> Next effect transition in
            </span>
            <span className="font-mono font-bold text-purple-300">{cycleRemain}s</span>
          </div>
          <div className="w-full h-1.5 bg-slate-950 rounded-full overflow-hidden border border-slate-800">
            <div
              className="h-full bg-gradient-to-r from-purple-500 via-cyan-400 to-pink-500 transition-all duration-300 rounded-full"
              style={{ width: `${cyclePct}%` }}
            />
          </div>
        </div>
      )}

      {/* Sliders: Speed & Auto Cycle Duration */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4 pt-2 border-t border-slate-800/80">
        {/* Speed Slider */}
        <div className="bg-slate-950/60 p-3.5 rounded-xl border border-slate-800/80 flex flex-col justify-between">
          <div className="flex items-center justify-between mb-2">
            <span className="text-xs font-bold text-slate-300 flex items-center gap-1.5">
              <Gauge className="w-4 h-4 text-cyan-400" /> Animation Speed
            </span>
            <span className="text-xs font-mono font-extrabold text-cyan-400 bg-cyan-950/50 px-2 py-0.5 rounded border border-cyan-800/40">
              {state.s} ms
            </span>
          </div>
          <input
            type="range"
            min={40}
            max={600}
            step={10}
            value={state.s}
            onChange={(e) => onSetSpeed(Number(e.target.value))}
            className="w-full accent-cyan-400 cursor-pointer h-2 bg-slate-800 rounded-lg"
          />
          <div className="flex justify-between text-[10px] text-slate-500 mt-1 font-semibold">
            <span>Fast (40ms)</span>
            <span>Balanced (120ms)</span>
            <span>Slow (600ms)</span>
          </div>
        </div>

        {/* Auto Cycle Duration Slider */}
        <div className="bg-slate-950/60 p-3.5 rounded-xl border border-slate-800/80 flex flex-col justify-between">
          <div className="flex items-center justify-between mb-2">
            <span className="text-xs font-bold text-slate-300 flex items-center gap-1.5">
              <Timer className="w-4 h-4 text-purple-400" /> Auto-Cycle Duration
            </span>
            <span className="text-xs font-mono font-extrabold text-purple-400 bg-purple-950/50 px-2 py-0.5 rounded border border-purple-800/40">
              {state.c} sec
            </span>
          </div>
          <input
            type="range"
            min={3}
            max={60}
            step={1}
            value={state.c}
            onChange={(e) => onSetCycle(Number(e.target.value))}
            className="w-full accent-purple-500 cursor-pointer h-2 bg-slate-800 rounded-lg"
          />
          <div className="flex justify-between text-[10px] text-slate-500 mt-1 font-semibold">
            <span>3s</span>
            <span>15s</span>
            <span>60s</span>
          </div>
        </div>
      </div>
    </div>
  );
};

