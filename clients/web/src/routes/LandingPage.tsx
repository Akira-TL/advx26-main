import { Download } from "lucide-react";

import { DownloadTerminal } from "../components/landing/DownloadTerminal";
import { FlowSection } from "../components/landing/FlowSection";
import { SiteNav } from "../components/landing/SiteNav";
import { ValuesSection } from "../components/landing/ValuesSection";
import { useLocale } from "../i18n/LocaleContext";
import { APK_URL } from "../lib/config";

export function LandingPage() {
  const { t } = useLocale();

  return (
    <div className="min-h-dvh overflow-x-hidden bg-[#000000] text-white">
      <SiteNav />

      <main className="relative bg-[#000000]">
        <section className="mx-auto max-w-[1120px] px-6 pb-14 pt-8 sm:px-8 sm:pb-16 sm:pt-10 md:px-12 md:pb-20 md:pt-12 lg:px-16 lg:pb-24 lg:pt-14">
          <div className="flex flex-col gap-8 sm:gap-10 md:grid md:grid-cols-[minmax(0,1.05fr)_minmax(260px,400px)] md:items-center md:gap-x-6 lg:gap-x-8">
            <div className="order-1 md:row-start-1">
              <p className="mb-4 inline-flex items-center gap-2 font-mono text-[10px] tracking-[0.14em] text-mint animate-glitchIn sm:mb-5 sm:text-[11px]">
                <span className="h-1.5 w-1.5 shrink-0 animate-blink rounded-full bg-mint" />
                {t.hero.badge}
              </p>

              <h1 className="max-w-[640px] font-display text-[clamp(2rem,6.5vw,4.25rem)] font-extrabold leading-[1.06] tracking-[-0.02em] text-textMain animate-glitchIn [animation-delay:60ms]">
                {t.hero.h1a}
                <br />
                {t.hero.h1b}
              </h1>

              <p className="mt-4 max-w-lg text-[15px] leading-7 text-textMuted sm:mt-6 sm:text-base md:text-[16px] md:leading-7 lg:text-[17px] lg:leading-8">
                {t.hero.p1}
                <br className="hidden sm:block" />
                {t.hero.p2}
              </p>
            </div>

            <div className="order-2 md:row-start-2">
              <div className="flex w-full flex-col gap-3 sm:w-auto sm:flex-row sm:flex-wrap">
                <a className="sp-btn sp-btn-primary group w-full sm:w-auto" href={APK_URL}>
                  <Download size={16} />
                  {t.hero.download}
                  <span className="hidden font-mono text-[10px] tracking-wider text-ink/70 group-hover:inline">
                    READY TO DOWNLOAD
                  </span>
                </a>
                <a className="sp-btn sp-btn-ghost w-full sm:w-auto" href="#principle">
                  {t.hero.learn}
                </a>
              </div>

              <div className="mt-4 space-y-1.5 font-mono text-[10px] tracking-[0.08em] text-textFaint sm:mt-5 sm:text-[11px] sm:tracking-[0.1em]">
                <p className="text-textMuted">ANDROID / VERSION 1.0.0 / DIRECT APK</p>
                <p>{t.hero.tip}</p>
              </div>
            </div>

            <div className="order-3 flex justify-center md:row-span-2 md:row-start-1 md:justify-end">
              <img
                src="/hero-soundpola.png"
                alt="SoundPola"
                width={654}
                height={1024}
                className="mx-auto block h-auto w-full max-w-[260px] select-none object-contain sm:max-w-[300px] md:mx-0 md:max-w-[360px] lg:max-w-[400px]"
                draggable={false}
                decoding="async"
              />
            </div>
          </div>
        </section>

        <FlowSection />
        <ValuesSection />
        <DownloadTerminal />
      </main>

      <footer className="border-t border-white/[0.09] pb-[env(safe-area-inset-bottom)]">
        <div className="mx-auto flex max-w-[1120px] flex-col gap-3 px-6 py-7 text-[11px] text-textFaint sm:px-8 sm:text-[12px] md:flex-row md:items-center md:justify-between md:gap-4 md:px-12 md:py-8 lg:px-16">
          <p>SoundPola {"\u00a9"} 2026</p>
          <p className="font-mono tracking-[0.06em] sm:tracking-[0.08em]">{t.footer.tagline}</p>
          <p className="font-mono tracking-[0.08em] sm:tracking-[0.1em]">VERSION 1.0.0 {"\u00b7"} ANDROID AVAILABLE</p>
        </div>
      </footer>
    </div>
  );
}
