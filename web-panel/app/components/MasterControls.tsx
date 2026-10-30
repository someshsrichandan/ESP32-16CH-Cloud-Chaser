"use client";

import React from "react";
import { DeviceState } from "../types";
import { Play, Pause, Shuffle, RefreshCw, ChevronLeft, ChevronRight, Gauge, Timer, Repeat } from "lucide-react";

interface MasterControlsProps {
  state: DeviceState;
  onTriggerAction: (action: string) => void;
  onSetSpeed: (speed: number) => void;
  onSetCycle: (seconds: number) => void;
  onSetRepeats: (repeats: number) => void;
  onSetCycMode: (mode: number) => void;
}

export const MasterControls: React.FC<MasterControlsProps> = ({
  state,
  onTriggerAction,
  onSetSpeed,
  onSetCycle,
  onSetRepeats,
  onSetCycMode,
}) => {
  const isRunning = state.r === 1;
  const isAuto = state.a === 1;
  const isShuffle = state.sh === 1;
  const isManual = state.m === 1;
  const cycMode = state.cyc_mode ?? 0; // 0 = Repeats, 1 = Timed
  const currentReps = state.reps ?? 3;
  const activeCycle = state.cur_rep ?? 1;

  // Auto cycle countdown percentage for timed mode
  const cycleTotal = state.c > 0 ? state.c : 8;
  const cycleRemain = state.cd >= 0 ? state.cd : 0;
  const cyclePct = isAuto ? Math.max(0, Math.min(100, ((cycleTotal - cycleRemain) / cycleTotal) * 100)) : 0;

  // Repeat cycle percentage
  const repPct = isAuto && cycMode === 0 ? Math.min(100, Math.round((activeCycle / currentReps) * 100)) : 0;

  return (
    <div className="bg-slate-900/60 border border-slate-800/80 rounded-2xl p-4 md:p-6 shadow-xl backdrop-blur-md">
      {/* Title & Mode Status */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-5">
        <div>
          <h2 className="text-base font-bold text-white tracking-wide uppercase flex items-center gap-2">
            <span>Play &amp; Continuous Controls</span>
          </h2>
          <p className="text-xs text-slate-400 mt-0.5 flex flex-wrap items-center gap-2">
            <span>
              Current: <b className="text-purple-300">{state.ename || `Effect ${state.e}`}</b>
            </span>
            {isAuto && (
              <span className="px-2 py-0.5 rounded text-[10px] font-bold bg-purple-950/60 text-purple-300 border border-purple-800/50">
                {cycMode === 0 ? `CYCLE ${activeCycle}/${currentReps}` : `TIMED ${cycleRemain}s`}
              </span>
            )}
            {isManual && (
              <span className="px-2 py-0.5 rounded text-[10px] font-bold bg-amber-500/20 text-amber-300 border border-amber-500/40">
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

      {/* Auto Mode Cycle Progress Bar */}
      {isAuto && (
        <div className="mb-5 bg-slate-950/70 p-3 rounded-xl border border-slate-800/80">
          <div className="flex items-center justify-between text-xs text-slate-400 mb-1.5 font-medium">
            <span className="flex items-center gap-1.5">
              <Repeat className="w-3.5 h-3.5 text-purple-400 animate-spin" />
              {cycMode === 0
                ? `Running Repeat Cycle ${activeCycle} of ${currentReps} (Smooth Zero-Delay Transition)`
                : `Timed auto-transition in ${cycleRemain}s`}
            </span>
            <span className="font-mono font-bold text-purple-300">
              {cycMode === 0 ? `${activeCycle}/${currentReps} Cycles` : `${cycleRemain}s`}
            </span>
          </div>
          <div className="w-full h-1.5 bg-slate-900 rounded-full overflow-hidden border border-slate-800">
            <div
              className="h-full bg-gradient-to-r from-purple-500 via-cyan-400 to-pink-500 transition-all duration-300 rounded-full"
              style={{ width: `${cycMode === 0 ? repPct : cyclePct}%` }}
            />
          </div>
        </div>
      )}

      {/* Repeat Cycles Presets & Sliders */}
      <div className="space-y-4 pt-2 border-t border-slate-800/80">
        {/* Cycle Mode Switch & Quick Presets */}
        <div className="bg-slate-950/70 p-3.5 rounded-xl border border-slate-800/80">
          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-2 mb-3">
            <div className="flex items-center gap-2">
              <span className="text-xs font-bold text-slate-200 uppercase tracking-wide flex items-center gap-1.5">
                <Repeat className="w-4 h-4 text-cyan-400" /> Repeat Cycles Per Effect
              </span>
              <span className="text-[10px] text-slate-400">
                (Runs each effect N times, then smoothly auto-advances)
              </span>
            </div>
            {/* Mode Switch Button */}
            <button
              onClick={() => onSetCycMode(cycMode === 0 ? 1 : 0)}
              className="px-2.5 py-1 rounded-lg text-[11px] font-bold bg-slate-800 hover:bg-slate-700 text-purple-300 border border-slate-700 self-start sm:self-auto transition-all"
            >
              Mode: {cycMode === 0 ? `Repeats (${currentReps}x)` : `Timed (${state.c}s)`}
            </button>
          </div>

          {/* Quick Repeat Buttons (1x, 2x, 3x, 4x, 5x, 8x, 10x) */}
          <div className="flex flex-wrap items-center gap-2">
            {[1, 2, 3, 4, 5, 8, 10].map((num) => (
              <button
                key={num}
                onClick={() => onSetRepeats(num)}
                className={`px-3 py-1.5 rounded-xl text-xs font-extrabold transition-all shadow-sm active:scale-95 ${
                  currentReps === num && cycMode === 0
                    ? "bg-purple-600 text-white border border-purple-400 shadow-purple-600/30 scale-105"
                    : "bg-slate-900 text-slate-300 border border-slate-800 hover:bg-slate-800 hover:text-white"
                }`}
              >
                {num}x Repeat
              </button>
            ))}
          </div>
        </div>

        {/* Sliders: Speed & Repeat/Time Slider */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
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

          {/* Repeat Slider OR Timed Slider */}
          {cycMode === 0 ? (
            <div className="bg-slate-950/60 p-3.5 rounded-xl border border-slate-800/80 flex flex-col justify-between">
              <div className="flex items-center justify-between mb-2">
                <span className="text-xs font-bold text-slate-300 flex items-center gap-1.5">
                  <Repeat className="w-4 h-4 text-purple-400" /> Custom Cycle Repeats
                </span>
                <span className="text-xs font-mono font-extrabold text-purple-400 bg-purple-950/50 px-2 py-0.5 rounded border border-purple-800/40">
                  {currentReps} Cycles
                </span>
              </div>
              <input
                type="range"
                min={1}
                max={20}
                step={1}
                value={currentReps}
                onChange={(e) => onSetRepeats(Number(e.target.value))}
                className="w-full accent-purple-500 cursor-pointer h-2 bg-slate-800 rounded-lg"
              />
              <div className="flex justify-between text-[10px] text-slate-500 mt-1 font-semibold">
                <span>1 cycle</span>
                <span>5 cycles</span>
                <span>20 cycles</span>
              </div>
            </div>
          ) : (
            <div className="bg-slate-950/60 p-3.5 rounded-xl border border-slate-800/80 flex flex-col justify-between">
              <div className="flex items-center justify-between mb-2">
                <span className="text-xs font-bold text-slate-300 flex items-center gap-1.5">
                  <Timer className="w-4 h-4 text-purple-400" /> Auto-Cycle Seconds
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
          )}
        </div>
      </div>
    </div>
  );
};

