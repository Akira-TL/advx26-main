import { useLocale } from "../../i18n/LocaleContext";

type Graphic = "orbit" | "id" | "trigger" | "trail";

function MiniGraphic({ type }: { type: Graphic }) {
  if (type === "orbit") {
    return (
      <svg viewBox="0 0 64 64" className="h-10 w-10 sm:h-12 sm:w-12" aria-hidden>
        <circle cx="32" cy="32" r="22" fill="none" stroke="rgba(99,224,203,0.35)" strokeWidth="1" />
        <circle cx="32" cy="32" r="10" fill="none" stroke="#63E0CB" strokeWidth="1" />
        <circle cx="32" cy="10" r="2" fill="#63E0CB" />
      </svg>
    );
  }
  if (type === "id") {
    return (
      <svg viewBox="0 0 64 64" className="h-10 w-10 sm:h-12 sm:w-12" aria-hidden>
        <rect x="12" y="18" width="40" height="28" fill="none" stroke="rgba(99,224,203,0.35)" strokeWidth="1" />
        <text x="20" y="36" fill="#63E0CB" fontSize="10" fontFamily="monospace">
          #0F2A
        </text>
      </svg>
    );
  }
  if (type === "trigger") {
    return (
      <svg viewBox="0 0 64 64" className="h-10 w-10 sm:h-12 sm:w-12" aria-hidden>
        <circle cx="32" cy="32" r="18" fill="none" stroke="rgba(99,224,203,0.35)" strokeWidth="1" />
        <circle cx="32" cy="32" r="5" fill="#63E0CB" />
        <path d="M32 8v8M32 48v8M8 32h8M48 32h8" stroke="#63E0CB" strokeWidth="1" />
      </svg>
    );
  }
  return (
    <svg viewBox="0 0 64 64" className="h-10 w-10 sm:h-12 sm:w-12" aria-hidden>
      <path d="M10 46 L24 28 L34 38 L54 14" fill="none" stroke="#63E0CB" strokeWidth="1.25" />
      <circle cx="54" cy="14" r="2.5" fill="#63E0CB" />
      <circle cx="24" cy="28" r="2" fill="rgba(244,246,245,0.7)" />
    </svg>
  );
}

export function ValuesSection() {
  const { t } = useLocale();
  const values = t.values.items;

  return (
    <section
      id="values"
      className="scroll-mt-20 border-t border-white/[0.09] py-16 sm:scroll-mt-24 sm:py-20 md:py-24 lg:py-[7.5rem]"
    >
      <div className="mx-auto max-w-[1120px] px-6 sm:px-8 md:px-12 lg:px-16">
        <p className="font-mono text-[11px] tracking-[0.16em] text-mint">02 / WHY</p>
        <h2 className="mt-3 font-display text-[clamp(1.5rem,4.5vw,2.25rem)] font-bold text-textMain sm:mt-4">
          {t.values.title}
        </h2>
        <p className="mt-3 max-w-2xl text-[15px] leading-7 text-textMuted sm:mt-4 sm:text-base">{t.values.lead}</p>

        <div className="mt-10 grid border-t border-white/[0.09] sm:mt-12 md:mt-14 md:grid-cols-2">
          {values.map((item, index) => (
            <article
              key={item.code}
              className={`group border-b border-white/[0.09] p-5 transition hover:bg-white/[0.02] sm:p-6 md:p-8 ${
                index % 2 === 0 ? "md:border-r" : ""
              }`}
            >
              <div className="flex items-start justify-between gap-4">
                <p className="font-mono text-[36px] leading-none tracking-tight text-mint/20 sm:text-[40px] md:text-[52px]">
                  {item.code}
                </p>
                <MiniGraphic type={item.graphic as Graphic} />
              </div>
              <h3 className="mt-5 text-lg font-semibold text-textMain transition group-hover:translate-x-px sm:mt-6 sm:text-xl">
                {item.title}
              </h3>
              <p className="mt-2 max-w-md text-sm leading-6 text-textMuted sm:mt-3">{item.body}</p>
              <div className="mt-5 h-px w-12 bg-mint/30 transition group-hover:w-20 group-hover:bg-mint/70 sm:mt-6" />
            </article>
          ))}
        </div>
      </div>
    </section>
  );
}
