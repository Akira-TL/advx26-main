export function formatDate(iso: string | null | undefined): string {
  return iso ? iso.slice(0, 10).replace(/-/g, ".") : "";
}

export function formatDurationMs(durationMs: number | null | undefined): string {
  return durationMs ? (durationMs / 1000).toFixed(1) : "—";
}

export function formatClock(seconds: number): string {
  const minutes = Math.floor(seconds / 60);
  const secs = Math.floor(seconds % 60);

  return `${minutes}:${secs < 10 ? "0" : ""}${secs}`;
}

export function shortAddress(address: string | null | undefined): string {
  return address && address.length >= 12
    ? `${address.slice(0, 6)}…${address.slice(-4)}`
    : "";
}
