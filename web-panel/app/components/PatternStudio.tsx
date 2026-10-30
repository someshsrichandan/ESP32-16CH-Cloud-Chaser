"use client";

import React, { useState, useEffect, useRef } from "react";
import { CustomPattern } from "../types";
import { Sliders, Play, Pause, UploadCloud, RotateCcw, Plus, Trash2, Wand2, Download } from "lucide-react";

interface PatternStudioProps {
  onUploadPattern: (pattern: CustomPattern) => void;
  onStreamFrame: (mask: number) => void;
}

// Pre-built custom presets (No flashing required)
const PRESETS: Record<string, { speed: number; frames: number[] }> = {
  "Double Laser Scan": {
    speed: 70,
    frames: [
      0x0003, 0x0006, 0x000C, 0x0018, 0x0030, 0x0060, 0x00C0, 0x0180,
      0x0300, 0x0600, 0x0C00, 0x1800, 0x3000, 0x6000, 0xC000, 0x8001,
      0x4002, 0x2004, 0x1008, 0x0810, 0x0420, 0x0240, 0x0180, 0x00C0
    ],
  },
  "Accordion In-Out": {
    speed: 90,
    frames: [
      0x0180, 0x03C0, 0x07E0, 0x0FF0, 0x1FF8, 0x3FFC, 0x7FFE, 0xFFFF,
      0x7FFE, 0x3FFC, 0x1FF8, 0x0FF0, 0x07E0, 0x03C0, 0x0180, 0x0000
    ],
  },
  "Binary Arpeggio": {
    speed: 100,
    frames: [
      0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
      0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000,
      0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xFFFF, 0x0000, 0xFFFF, 0x0000
    ],
  },
  "Waterfall Cascade": {
    speed: 60,
    frames: [
      0x8000, 0xC000, 0xE000, 0xF000, 0x7800, 0x3C00, 0x1E00, 0x0F00,
      0x0780, 0x03C0, 0x01E0, 0x00F0, 0x0078, 0x003C, 0x001E, 0x000F,
      0x0007, 0x0003, 0x0001, 0x0000
    ],
  },
  "Eurodance Strobe 140BPM": {
    speed: 100,
    frames: [
      0x00FF, 0x0000, 0x00FF, 0x0000, 0xFF00, 0x0000, 0xFF00, 0x0000,
      0x3333, 0xCCCC, 0x3333, 0xCCCC, 0xFFFF, 0x0000, 0xFFFF, 0x0000
    ],
  },
};

export const PatternStudio: React.FC<PatternStudioProps> = ({
  onUploadPattern,
  onStreamFrame,
}) => {
  const [patternName, setPatternName] = useState("Custom Light Show");
  const [speed, setSpeed] = useState(100);
  const [stepCount, setStepCount] = useState(16);
  // Grid matrix: steps × 16 relays
  const [grid, setGrid] = useState<boolean[][]>(() =>
    Array.from({ length: 16 }, () => Array(16).fill(false))
  );

  const [isPlayingPreview, setIsPlayingPreview] = useState(false);
  const [currentPreviewStep, setCurrentPreviewStep] = useState(0);
  const [isPushing, setIsPushing] = useState(false);
  const [pushSuccess, setPushSuccess] = useState(false);

  const previewTimerRef = useRef<NodeJS.Timeout | null>(null);

  // Initialize with a default pattern on first load
  useEffect(() => {
    loadPreset("Double Laser Scan");
  }, []);

  // Browser Preview Sequencer
  useEffect(() => {
    if (!isPlayingPreview) {
      if (previewTimerRef.current) clearInterval(previewTimerRef.current);
      return;
    }

    previewTimerRef.current = setInterval(() => {
      setCurrentPreviewStep((prev) => {
        const next = (prev + 1) % stepCount;
        // Calculate 16-bit mask
        let mask = 0;
        for (let ch = 0; ch < 16; ch++) {
          if (grid[next]?.[ch]) mask |= 1 << ch;
        }
        // Optional live frame stream
        onStreamFrame(mask);
        return next;
      });
    }, speed);

    return () => {
      if (previewTimerRef.current) clearInterval(previewTimerRef.current);
    };
  }, [isPlayingPreview, stepCount, speed, grid, onStreamFrame]);

  const toggleCell = (stepIdx: number, chIdx: number) => {
    setGrid((prev) => {
      const next = prev.map((step) => [...step]);
      if (!next[stepIdx]) next[stepIdx] = Array(16).fill(false);
      next[stepIdx][chIdx] = !next[stepIdx][chIdx];
      return next;
    });
  };

  const loadPreset = (presetKey: string) => {
    const p = PRESETS[presetKey];
    if (!p) return;

    setPatternName(presetKey);
    setSpeed(p.speed);
    setStepCount(p.frames.length);

    const newGrid: boolean[][] = [];
    for (let s = 0; s < p.frames.length; s++) {
      const mask = p.frames[s];
      const stepChannels: boolean[] = [];
      for (let ch = 0; ch < 16; ch++) {
        stepChannels.push(((mask >> ch) & 1) === 1);
      }
      newGrid.push(stepChannels);
    }
    setGrid(newGrid);
  };

  const clearGrid = () => {
    setGrid(Array.from({ length: stepCount }, () => Array(16).fill(false)));
  };

  const addStep = () => {
    if (stepCount >= 32) return;
    setStepCount((prev) => prev + 1);
    setGrid((prev) => [...prev, Array(16).fill(false)]);
  };

  const removeStep = () => {
    if (stepCount <= 4) return;
    setStepCount((prev) => prev - 1);
    setGrid((prev) => prev.slice(0, prev.length - 1));
  };

  // Convert grid to 16-bit integer array & upload to ESP32
  const handlePushToESP32 = () => {
    setIsPushing(true);
    setPushSuccess(false);

    const frames: number[] = [];
    for (let s = 0; s < stepCount; s++) {
      let mask = 0;
      for (let ch = 0; ch < 16; ch++) {
        if (grid[s]?.[ch]) mask |= 1 << ch;
      }
      frames.push(mask);
    }

    onUploadPattern({
      name: patternName || "Custom FX",
      speed,
      frames,
    });

    setTimeout(() => {
      setIsPushing(false);
      setPushSuccess(true);
      setTimeout(() => setPushSuccess(false), 3000);
    }, 600);
  };

  return (
    <div className="bg-slate-900/60 border border-slate-800/80 rounded-2xl p-4 md:p-6 shadow-xl backdrop-blur-md">
      {/* Title & Preset Selector */}
      <div className="flex flex-col lg:flex-row lg:items-center justify-between gap-4 mb-6">
        <div>
          <div className="flex items-center gap-2">
            <Sliders className="w-5 h-5 text-cyan-400" />
            <h2 className="text-base font-bold text-white tracking-wide uppercase">
              Dynamic Effect Studio (No Re-Flashing Needed)
            </h2>
          </div>
          <p className="text-xs text-slate-400 mt-1">
            Build custom light shows in your browser and upload them directly to the ESP32 without Arduino IDE!
          </p>
        </div>

        {/* Preset Selector */}
        <div className="flex flex-wrap items-center gap-2">
          <span className="text-xs text-slate-400 font-semibold flex items-center gap-1">
            <Wand2 className="w-3.5 h-3.5 text-purple-400" /> Presets:
          </span>
          <select
            onChange={(e) => loadPreset(e.target.value)}
            className="bg-slate-950 border border-slate-700 text-xs text-slate-200 rounded-xl px-3 py-1.5 focus:outline-none focus:border-cyan-400"
          >
            {Object.keys(PRESETS).map((k) => (
              <option key={k} value={k}>
                {k}
              </option>
            ))}
          </select>
        </div>
      </div>

      {/* Pattern Settings Header */}
      <div className="grid grid-cols-1 sm:grid-cols-3 gap-3 mb-4 p-3 bg-slate-950/70 border border-slate-800 rounded-xl">
        {/* Name input */}
        <div>
          <label className="text-[11px] font-bold text-slate-400 mb-1 block">Pattern Name</label>
          <input
            type="text"
            value={patternName}
            onChange={(e) => setPatternName(e.target.value)}
            className="w-full bg-slate-900 border border-slate-700 rounded-lg px-2.5 py-1 text-xs text-white focus:outline-none focus:border-cyan-400"
          />
        </div>

        {/* Step Speed */}
        <div>
          <label className="text-[11px] font-bold text-slate-400 mb-1 block">
            Step Interval: <span className="text-cyan-400 font-mono font-bold">{speed} ms</span>
          </label>
          <input
            type="range"
            min={40}
            max={500}
            step={10}
            value={speed}
            onChange={(e) => setSpeed(Number(e.target.value))}
            className="w-full accent-cyan-400 h-1.5 bg-slate-800 rounded cursor-pointer"
          />
        </div>

        {/* Step Controls */}
        <div className="flex items-end gap-2">
          <button
            onClick={() => setIsPlayingPreview(!isPlayingPreview)}
            className={`flex-1 py-1.5 px-3 rounded-lg text-xs font-bold flex items-center justify-center gap-1.5 transition-all shadow-md active:scale-95 ${
              isPlayingPreview
                ? "bg-amber-600 text-white"
                : "bg-blue-600 hover:bg-blue-500 text-white"
            }`}
          >
            {isPlayingPreview ? (
              <>
                <Pause className="w-3.5 h-3.5 fill-current" /> Stop Preview
              </>
            ) : (
              <>
                <Play className="w-3.5 h-3.5 fill-current" /> Preview Live
              </>
            )}
          </button>

          <button
            onClick={addStep}
            disabled={stepCount >= 32}
            className="p-1.5 bg-slate-800 border border-slate-700 hover:bg-slate-700 text-slate-300 rounded-lg text-xs font-bold disabled:opacity-40"
            title="Add Step"
          >
            <Plus className="w-4 h-4" />
          </button>
          <button
            onClick={removeStep}
            disabled={stepCount <= 4}
            className="p-1.5 bg-slate-800 border border-slate-700 hover:bg-slate-700 text-slate-300 rounded-lg text-xs font-bold disabled:opacity-40"
            title="Remove Step"
          >
            <Trash2 className="w-4 h-4" />
          </button>
          <button
            onClick={clearGrid}
            className="p-1.5 bg-slate-800 border border-slate-700 hover:bg-slate-700 text-slate-300 rounded-lg text-xs font-bold"
            title="Clear Matrix"
          >
            <RotateCcw className="w-4 h-4" />
          </button>
        </div>
      </div>

      {/* 16-Channel Sequencer Matrix */}
      <div className="overflow-x-auto pb-2 mb-5">
        <div className="inline-block min-w-full">
          <table className="border-collapse">
            <thead>
              <tr>
                <th className="p-1 text-[10px] text-slate-500 font-mono text-left w-12 sticky left-0 bg-slate-900/90 backdrop-blur z-10">
                  CH
                </th>
                {Array.from({ length: stepCount }).map((_, stepIdx) => (
                  <th
                    key={stepIdx}
                    className={`p-1 text-[10px] font-mono text-center min-w-[28px] ${
                      isPlayingPreview && currentPreviewStep === stepIdx
                        ? "text-cyan-300 font-bold bg-cyan-950/60 rounded-t"
                        : "text-slate-500"
                    }`}
                  >
                    {stepIdx + 1}
                  </th>
                ))}
              </tr>
            </thead>
            <tbody>
              {Array.from({ length: 16 }).map((_, chIdx) => (
                <tr key={chIdx} className="border-b border-slate-800/40">
                  <td className="p-1 text-[11px] font-mono font-bold text-slate-400 sticky left-0 bg-slate-900/90 backdrop-blur z-10">
                    CH{chIdx + 1}
                  </td>
                  {Array.from({ length: stepCount }).map((_, stepIdx) => {
                    const isCellOn = !!grid[stepIdx]?.[chIdx];
                    const isCurrentStep = isPlayingPreview && currentPreviewStep === stepIdx;
                    return (
                      <td
                        key={stepIdx}
                        onClick={() => toggleCell(stepIdx, chIdx)}
                        className={`p-1 cursor-pointer transition-all duration-75 text-center ${
                          isCurrentStep ? "bg-cyan-950/30" : ""
                        }`}
                      >
                        <div
                          className={`w-6 h-6 rounded-md border transition-all ${
                            isCellOn
                              ? "bg-cyan-500 border-cyan-300 shadow-md shadow-cyan-500/50 scale-105"
                              : "bg-slate-950/90 border-slate-800 hover:border-slate-600"
                          }`}
                        />
                      </td>
                    );
                  })}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Upload Action */}
      <div className="flex flex-col sm:flex-row items-center justify-between gap-3 p-3 bg-gradient-to-r from-purple-950/40 to-cyan-950/40 border border-purple-800/40 rounded-xl">
        <div className="text-xs text-slate-300">
          <span className="font-bold text-white">Ready to Play?</span> Push this sequence directly to your ESP32.
          It will run immediately and persist in non-volatile flash memory!
        </div>

        <button
          onClick={handlePushToESP32}
          disabled={isPushing}
          className={`px-5 py-2.5 rounded-xl text-xs font-black flex items-center gap-2 transition-all shadow-xl active:scale-95 whitespace-nowrap ${
            pushSuccess
              ? "bg-emerald-600 text-white shadow-emerald-500/30"
              : "bg-gradient-to-r from-purple-600 to-cyan-600 hover:from-purple-500 hover:to-cyan-500 text-white shadow-purple-600/30"
          }`}
        >
          {isPushing ? (
            <>Uploading to ESP32...</>
          ) : pushSuccess ? (
            <>✓ Running Live on ESP32!</>
          ) : (
            <>
              <UploadCloud className="w-4 h-4" /> Push Pattern to ESP32
            </>
          )}
        </button>
      </div>
    </div>
  );
};

