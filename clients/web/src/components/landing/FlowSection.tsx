import { useEffect, useRef, useState } from "react";

import { useLocale } from "../../i18n/LocaleContext";

export function FlowSection() {
  const { t } = useLocale();
  const steps = t.flow.steps;
  const ref = useRef<HTMLElement>(null);
  const [progress, setProgress] = useState(0);
  const [hover, setHover] = useState<number | null>(null);

  useEffect(() => {
    const el = ref.current;
    if (!el) return;
    const onScroll = () => {
      const rect = el.getBoundingClientRect();
      const view = window.innerHeight || 1;
      const raw = (view * 0.75 - rect.top) / (view * 0.5 + rect.height * 0.35);
      setProgress(Math.min(1, Math.max(0, raw)));
    };
    onScroll();
    window.addEventListener("scroll", onScroll, { passive: true });
    return () => window.removeEventListener("scroll", onScroll);
  }, []);

  const activeCount = hover !== null ? hover + 1 : Math.round(progress * (steps.length - 1)) + 1;

  return (
    <section
      id="principle"
      ref={ref}
      className="scroll-mt-20 border-t border-white/[0.09] py-16 sm:scroll-mt-24 sm:py-20 md:py-24 lg:py-[7.5rem]"
    >
      <div className="mx-auto max-w-[1120px] px-6 sm:px-8 md:px-12 lg:px-16">
        <p className="font-mono text-[11px] tracking-[0.16em] text-mint">01 / FLOW</p>
        <h2 className="mt-3 max-w-3xl font-display text-[clamp(1.5rem,4.5vw,2.25rem)] font-bold leading-tight text-textMain sm:mt-4">
          {t.flow.title}
        </h2>
        <p className="mt-3 max-w-2xl text-[15px] leading-7 text-textMuted sm:mt-4 sm:text-base">{t.flow.lead}</p>
        <p className="mt-3 max-w-2xl font-mono text-[10px] leading-5 tracking-[0.06em] text-textFaint sm:text-[11px]">
          {t.flow.note}
        </p>

        <div className="relative mt-10 sm:mt-12 lg:mt-14">
          <div className="pointer-events-none absolute left-0 right-0 top-[15px] hidden h-px bg-white/[0.09] lg:block" />
          <div
            className="pointer-events-none absolute left-0 top-[15px] hidden h-px bg-mint transition-[width] duration-500 lg:block"
            style={{ width: `${((Math.max(activeCount, 1) - 1) / (steps.length - 1)) * 100}%` }}
          />

          <ol className="relative grid grid-cols-1 gap-0 border-l border-white/[0.09] pl-5 sm:pl-6 md:grid-cols-2 md:gap-x-6 md:gap-y-8 md:border-l-0 md:pl-0 lg:grid-cols-4 lg:gap-6">
            {steps.map((step, i) => {
              const on = i < activeCount;
              return (
                <li
                  key={step.code}
                  className="group relative border-transparent pb-8 last:pb-0 md:border-t md:border-white/[0.09] md:pb-0 md:pt-5 lg:border-transparent lg:pt-10 lg:hover:border-mint/40"
                  onMouseEnter={() => setHover(i)}
                  onMouseLeave={() => setHover(null)}
                >
                  <span
                    className={`absolute -left-[25px] top-1 h-[11px] w-[11px] rounded-full border sm:-left-[29px] md:static md:mb-3 md:inline-block lg:absolute lg:left-0 lg:top-[10px] lg:mb-0 ${
                      on ? "border-mint bg-mint" : "border-white/20 bg-black"
                    }`}
                  />
                  <p className="font-mono text-2xl font-medium tracking-tight text-mint/25 sm:text-3xl lg:text-4xl">
                    {step.code}
                  </p>
                  <h3 className="mt-2 text-base font-semibold text-textMain sm:mt-3 lg:mt-4 lg:text-lg">
                    {step.title}
                  </h3>
                  <p className="mt-2 text-sm leading-6 text-textMuted lg:mt-3">{step.body}</p>
                </li>
              );
            })}
          </ol>
        </div>
      </div>
    </section>
  );
}
