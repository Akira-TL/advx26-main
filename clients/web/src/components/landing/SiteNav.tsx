import { useEffect, useState } from "react";

import { useLocale } from "../../i18n/LocaleContext";

export function SiteNav() {
  const { t, locale, setLocale } = useLocale();
  const [open, setOpen] = useState(false);
  const [scrolled, setScrolled] = useState(false);
  const [active, setActive] = useState("");

  const links = [
    { href: "#principle", label: t.nav.principle, id: "principle" },
    { href: "#values", label: t.nav.values, id: "values" },
    { href: "#download", label: t.nav.download, id: "download" },
  ] as const;

  useEffect(() => {
    const onScroll = () => {
      setScrolled(window.scrollY > 12);
      const ids = ["principle", "values", "download"] as const;
      let current = "";
      for (const id of ids) {
        const el = document.getElementById(id);
        if (!el) continue;
        if (el.getBoundingClientRect().top <= 120) current = id;
      }
      setActive(current);
    };
    onScroll();
    window.addEventListener("scroll", onScroll, { passive: true });
    return () => window.removeEventListener("scroll", onScroll);
  }, []);

  useEffect(() => {
    if (!open) return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") setOpen(false);
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [open]);

  const LangSwitch = ({ className = "" }: { className?: string }) => (
    <div
      className={`inline-flex items-center rounded-lg border border-white/[0.09] p-0.5 font-mono text-[11px] tracking-[0.08em] ${className}`}
      role="group"
      aria-label="Language"
    >
      <button
        type="button"
        className={`rounded-md px-2.5 py-1.5 transition ${
          locale === "zh" ? "bg-mint text-ink" : "text-textMuted hover:text-textMain"
        }`}
        aria-pressed={locale === "zh"}
        onClick={() => setLocale("zh")}
      >
        {"\u4e2d"}
      </button>
      <button
        type="button"
        className={`rounded-md px-2.5 py-1.5 transition ${
          locale === "en" ? "bg-mint text-ink" : "text-textMuted hover:text-textMain"
        }`}
        aria-pressed={locale === "en"}
        onClick={() => setLocale("en")}
      >
        EN
      </button>
    </div>
  );

  return (
    <header
      className={`sticky top-0 z-40 border-b pt-[env(safe-area-inset-top)] transition-colors ${
        scrolled ? "border-white/[0.09] bg-black/95 backdrop-blur-md" : "border-transparent bg-black"
      }`}
    >
      <div className="mx-auto flex h-14 max-w-[1120px] items-center justify-between gap-3 px-6 sm:h-16 sm:gap-4 sm:px-8 md:h-[72px] md:px-12 lg:px-16">
        <a href="#" className="inline-flex min-w-0 items-center gap-2 no-underline sm:gap-2.5">
          <span className="grid h-8 w-8 shrink-0 place-items-center rounded-lg bg-mint text-ink">
            <span className="font-display text-sm font-extrabold leading-none">SP</span>
          </span>
          <span className="truncate font-display text-base font-bold tracking-wide text-textMain sm:text-lg">
            SoundPola
          </span>
          <span className="hidden shrink-0 font-mono text-[10px] tracking-[0.14em] text-mint sm:inline">
            BETA / 1.0.0
          </span>
        </a>

        <div className="hidden items-center gap-5 md:flex lg:gap-7">
          <nav className="flex items-center gap-6 lg:gap-8">
            {links.map((link) => (
              <a
                key={link.href}
                href={link.href}
                className={`font-mono text-[12px] tracking-[0.12em] no-underline transition ${
                  active === link.id ? "text-mint" : "text-textMuted hover:text-mint"
                }`}
              >
                {link.label}
              </a>
            ))}
          </nav>
          <LangSwitch />
        </div>

        <div className="flex items-center gap-2 md:hidden">
          <LangSwitch />
          <button
            type="button"
            className="grid h-9 w-9 shrink-0 place-items-center rounded-lg border border-white/[0.09] text-textMain"
            aria-label={open ? t.nav.closeMenu : t.nav.openMenu}
            aria-expanded={open}
            onClick={() => setOpen((v) => !v)}
          >
            <span className="font-mono text-lg leading-none">{open ? "\u00d7" : "\u2261"}</span>
          </button>
        </div>
      </div>

      {open && (
        <div className="border-t border-white/[0.09] bg-black px-6 py-4 sm:px-8 md:hidden">
          <div className="flex flex-col gap-1">
            {links.map((link) => (
              <a
                key={link.href}
                href={link.href}
                className={`rounded-lg px-2 py-3 font-mono text-sm tracking-wider no-underline ${
                  active === link.id ? "text-mint" : "text-textMain"
                }`}
                onClick={() => setOpen(false)}
              >
                {link.label}
              </a>
            ))}
          </div>
        </div>
      )}
    </header>
  );
}
