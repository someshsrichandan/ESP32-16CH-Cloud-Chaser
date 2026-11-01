import type { Config } from "tailwindcss"

const config: Config = {
  darkMode: ["class"],
  content: [
    "./src/pages/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/components/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/app/**/*.{js,ts,jsx,tsx,mdx}",
  ],
  theme: {
    container: {
      center: true,
      padding: "2rem",
      screens: {
        "2xl": "1400px",
      },
    },
    extend: {
      colors: {
        border: "hsl(var(--border, 240 3.7% 15.9%))",
        input: "hsl(var(--input, 240 3.7% 15.9%))",
        ring: "hsl(var(--ring, 187 92% 55%))",
        background: "#0a0a0f",
        foreground: "hsl(var(--foreground, 0 0% 98%))",
        "bg-page": "#0a0a0f",
        "bg-card": "#12121a",
        "bg-card-hover": "#181824",
        "relay-on": "#22d3ee",
        "relay-off": "#27272a",
        accent: {
          DEFAULT: "#22d3ee",
          foreground: "#09090b",
          hover: "#06b6d4",
          muted: "rgba(34, 211, 238, 0.15)",
        },
        warning: {
          DEFAULT: "#fbbf24",
          foreground: "#09090b",
          muted: "rgba(251, 191, 36, 0.15)",
        },
        danger: {
          DEFAULT: "#ef4444",
          foreground: "#ffffff",
          muted: "rgba(239, 68, 68, 0.15)",
        },
        success: {
          DEFAULT: "#34d399",
          foreground: "#09090b",
          muted: "rgba(52, 211, 153, 0.15)",
        },
        primary: {
          DEFAULT: "#22d3ee",
          foreground: "#09090b",
        },
        secondary: {
          DEFAULT: "#1e1e2d",
          foreground: "#e4e4e7",
        },
        muted: {
          DEFAULT: "#27272a",
          foreground: "#a1a1aa",
        },
        destructive: {
          DEFAULT: "#ef4444",
          foreground: "#fafafa",
        },
        card: {
          DEFAULT: "#12121a",
          foreground: "#f4f4f5",
        },
        popover: {
          DEFAULT: "#12121a",
          foreground: "#f4f4f5",
        },
      },
      borderRadius: {
        lg: "0.625rem",
        md: "0.5rem",
        sm: "0.375rem",
      },
      boxShadow: {
        "glow-cyan": "0 0 15px rgba(34, 211, 238, 0.5), 0 0 30px rgba(34, 211, 238, 0.25)",
        "glow-cyan-sm": "0 0 8px rgba(34, 211, 238, 0.6)",
        "glow-cyan-lg": "0 0 25px rgba(34, 211, 238, 0.7), 0 0 50px rgba(34, 211, 238, 0.35)",
        "glow-amber": "0 0 15px rgba(251, 191, 36, 0.5)",
        "glow-red": "0 0 15px rgba(239, 68, 68, 0.6)",
        "glow-green": "0 0 15px rgba(52, 211, 153, 0.5)",
      },
      keyframes: {
        "pulse-glow": {
          "0%, 100%": { opacity: "1", transform: "scale(1)" },
          "50%": { opacity: "0.6", transform: "scale(0.97)" },
        },
        "radar-sweep": {
          "0%": { transform: "rotate(0deg)" },
          "100%": { transform: "rotate(360deg)" },
        },
        "chaser-pingpong": {
          "0%, 100%": { transform: "translateX(0%)" },
          "50%": { transform: "translateX(100%)" },
        },
      },
      animation: {
        "pulse-glow": "pulse-glow 2s cubic-bezier(0.4, 0, 0.6, 1) infinite",
        "radar-sweep": "radar-sweep 3s linear infinite",
        "chaser-pingpong": "chaser-pingpong 2s ease-in-out infinite",
      },
    },
  },
  plugins: [],
}

export default config
