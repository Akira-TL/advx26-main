import * as Tooltip from "@radix-ui/react-tooltip";
import type { ReactNode } from "react";

interface ActionTooltipProps {
  label: string;
  children: ReactNode;
}

export function ActionTooltip({ label, children }: ActionTooltipProps) {
  return (
    <Tooltip.Root>
      <Tooltip.Trigger asChild>{children}</Tooltip.Trigger>
      <Tooltip.Portal>
        <Tooltip.Content
          sideOffset={8}
          className="z-50 rounded-md border border-white/10 bg-panelSoft px-3 py-2 text-xs text-textMain shadow-xl"
        >
          {label}
          <Tooltip.Arrow className="fill-panelSoft" />
        </Tooltip.Content>
      </Tooltip.Portal>
    </Tooltip.Root>
  );
}
