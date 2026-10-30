"use client";

import React, { useState } from "react";
import { Sparkles, Search } from "lucide-react";
import { EFFECT_NAMES } from "../hooks/useRelayController";

interface EffectsGridProps {
  currentEffect: number;
  onSelectEffect: (id: number) => void;
}

export const EffectsGrid: React.FC<EffectsGridProps> = ({
  currentEffect,
  onSelectEffect,
}) => {
  const [search, setSearch] = useState("");

  const filteredEffects = EFFECT_NAMES.map((name, id) => ({ name, id })).filter((item) =>
    item.name.toLowerCase().includes(search.toLowerCase())
  );

  return (
    <div className="bg-slate-900/60 border border-slate-800/80 rounded-2xl p-4 md:p-6 shadow-xl backdrop-blur-md">
      {/* Header & Search */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-5">
        <div className="flex items-center gap-2">
          <Sparkles className="w-5 h-5 text-purple-400" />
          <h2 className="text-base font-bold text-white tracking-wide uppercase">
            Built-in Light Effects ({EFFECT_NAMES.length})
          </h2>
        </div>

        {/* Search Input */}
        <div className="relative">
          <Search className="w-4 h-4 text-slate-500 absolute left-3 top-1/2 -translate-y-1/2" />
          <input
            type="text"
            placeholder="Search effects..."
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            className="w-full sm:w-56 pl-9 pr-3 py-1.5 bg-slate-950 border border-slate-800 rounded-xl text-xs text-white placeholder-slate-500 focus:outline-none focus:border-purple-500 transition-all"
          />
        </div>
      </div>

      {/* Grid */}
      <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 gap-2 max-h-[360px] overflow-y-auto pr-1">
        {filteredEffects.map((item) => {
          const isActive = currentEffect === item.id;
          return (
            <button
              key={item.id}
              onClick={() => onSelectEffect(item.id)}
              className={`text-left px-3 py-2.5 rounded-xl border text-xs font-bold transition-all user-select-none truncate flex items-center justify-between gap-1.5 ${
                isActive
                  ? "bg-gradient-to-r from-purple-600 to-indigo-600 border-purple-400 text-white shadow-lg shadow-purple-600/30 scale-[1.02]"
                  : "bg-slate-950/70 border-slate-800/70 text-slate-300 hover:border-slate-700 hover:bg-slate-800/60"
              } active:scale-95`}
            >
              <span className="truncate">
                <span className="text-[10px] text-slate-500 font-mono mr-1.5">
                  #{item.id + 1}
                </span>
                {item.name}
              </span>
              {isActive && (
                <span className="w-2 h-2 rounded-full bg-cyan-300 shadow-sm shadow-cyan-300 animate-pulse flex-shrink-0" />
              )}
            </button>
          );
        })}
      </div>
    </div>
  );
};

