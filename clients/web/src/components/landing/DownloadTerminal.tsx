import { Download } from "lucide-react";

import { useLocale } from "../../i18n/LocaleContext";
import { APK_URL } from "../../lib/config";

export function DownloadTerminal() {
  const { t } = useLocale();

  const metas = [
    { label: "PLATFORM", value: "Android" },
    { label: "VERSION", value: "1.0.0" },
    { label: "FORMAT", value: "Direct APK" },
    { label: "STATUS", value: "Available" },
    { label: "iOS", value: t.download.iosMeta },
  ] as const;

  return (
    <section
      id="download"
      className="scroll-mt-20 border-t border-white/[0.09] py-16 sm:scroll-mt-24 sm:py-20 md:py-24 lg:py-[7.5rem]"
    >
      <div className="mx-auto max-w-[1120px] px-6 sm:px-8 md:px-12 lg:px-16">
        <p className="font-mono text-[11px] tracking-[0.16em] text-mint">03 / DOWNLOAD</p>
        <h2 className="mt-3 font-display text-[clamp(1.5rem,4.5vw,2.25rem)] font-bold text-textMain sm:mt-4">
          {t.download.title}
        </h2>
        <p className="mt-3 max-w-xl text-[15px] leading-7 text-textMuted sm:mt-4 sm:text-base">{t.download.lead}</p>

        <div className="relative mt-8 overflow-hidden rounded-[10px] border border-[rgba(99,224,203,0.16)] bg-black p-4 sm:mt-10 sm:p-6 md:mt-12 md:p-10">
          <span className="pointer-events-none absolute left-2 top-2 h-3 w-3 border-l border-t border-mint/50 sm:left-3 sm:top-3" />
          <span className="pointer-events-none absolute right-2 top-2 h-3 w-3 border-r border-t border-mint/50 sm:right-3 sm:top-3" />
          <span className="pointer-events-none absolute bottom-2 left-2 h-3 w-3 border-b border-l border-mint/50 sm:bottom-3 sm:left-3" />
          <span className="pointer-events-none absolute bottom-2 right-2 h-3 w-3 border-b border-r border-mint/50 sm:bottom-3 sm:right-3" />

          <div className="relative grid gap-6 sm:gap-8 lg:grid-cols-[1.2fr_1fr] lg:items-start">
            <div>
              <p className="font-mono text-[11px] tracking-[0.16em] text-mint">DOWNLOAD TERMINAL</p>
              <div className="mt-5 flex flex-col gap-3 sm:mt-6 sm:flex-row sm:flex-wrap">
                <a className="sp-btn sp-btn-primary group w-full sm:w-auto" href={APK_URL}>
                  <Download size={16} />
                  {t.download.apk}
                  <span className="hidden font-mono text-[10px] tracking-wider text-ink/70 group-hover:inline">
                    READY TO DOWNLOAD
                  </span>
                </a>
                <button type="button" className="sp-btn sp-btn-ghost sp-btn-disabled w-full sm:w-auto" disabled>
                  {t.download.ios}
                </button>
              </div>
              <p className="mt-4 inline-flex items-center gap-2 font-mono text-[11px] tracking-[0.12em] text-mint">
                <span className="h-1.5 w-1.5 shrink-0 animate-blink rounded-full bg-mint" />
                DOWNLOAD READY
              </p>
              <p className="mt-3 text-xs leading-5 text-textFaint">{t.download.tip}</p>
            </div>

            <dl className="relative grid grid-cols-1 gap-2.5 sm:grid-cols-2 sm:gap-3">
              {metas.map((item, index) => (
                <div
                  key={item.label}
                  className={`border border-white/[0.09] bg-black px-3 py-3 ${
                    index === metas.length - 1 ? "sm:col-span-2" : ""
                  }`}
                >
                  <dt className="font-mono text-[10px] tracking-[0.14em] text-textFaint">{item.label}</dt>
                  <dd className="mt-1.5 break-words text-sm text-textMain">{item.value}</dd>
                </div>
              ))}
            </dl>
          </div>
        </div>
      </div>
    </section>
  );
}
