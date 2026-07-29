import { Music2 } from "lucide-react";

interface BrandMarkProps {
  size?: "sm" | "lg";
}

export function BrandMark({ size = "sm" }: BrandMarkProps) {
  const isLarge = size === "lg";

  return (
    <span
      className={[
        "inline-flex shrink-0 items-center justify-center bg-mint text-ink",
        isLarge ? "h-14 w-14 rounded-xl" : "h-8 w-8 rounded-lg",
      ].join(" ")}
      aria-hidden="true"
    >
      <Music2 size={isLarge ? 28 : 17} strokeWidth={2.5} />
    </span>
  );
}
