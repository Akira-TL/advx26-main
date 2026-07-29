/** @type {import('tailwindcss').Config} */
export default {
  content: ["./index.html", "./src/**/*.{ts,tsx}"],
  theme: {
    extend: {
      colors: {
        ink: "#000000",
        panel: "#000000",
        deep: "#1A1A1A",
        mint: "#63E0CB",
        jade: "#3db8a5",
        textMain: "#FFFFFF",
        textMuted: "#9A9A9A",
        textFaint: "#666666",
        panelSoft: "#000000",
        gold: "#f0c060",
        coral: "#ff8f70",
      },
      maxWidth: {
        site: "1120px",
      },
      fontFamily: {
        display: ['"Syne"', "Noto Sans SC", "sans-serif"],
        sans: ['"Noto Sans SC"', "-apple-system", "BlinkMacSystemFont", "Segoe UI", "sans-serif"],
        mono: ['"IBM Plex Mono"', "SF Mono", "Menlo", "monospace"],
      },
      keyframes: {
        breathe: {
          "0%, 100%": { transform: "scale(1)" },
          "50%": { transform: "scale(1.035)" },
        },
        spinSlow: {
          "0%": { transform: "rotate(0deg)" },
          "100%": { transform: "rotate(360deg)" },
        },
        blink: {
          "0%, 100%": { opacity: "1" },
          "50%": { opacity: "0.35" },
        },
        glitchIn: {
          "0%": { opacity: "0", transform: "translate(2px, -1px)" },
          "40%": { opacity: "1", transform: "translate(-1px, 1px)" },
          "100%": { opacity: "1", transform: "translate(0, 0)" },
        },
      },
      animation: {
        breathe: "breathe 8s ease-in-out infinite",
        spinSlow: "spinSlow 48s linear infinite",
        blink: "blink 2.4s ease-in-out infinite",
        glitchIn: "glitchIn 0.55s ease-out both",
      },
    },
  },
  plugins: [],
};
