"use client";

import { useEffect, useRef, useState } from "react";
import { dict, type Lang, REPO_URL, RELEASES_URL } from "@/lib/i18n";

const otherLang = (l: Lang): Lang => (l === "en" ? "es" : "en");

function Reveal({
  children,
  delay = 0,
  className = "",
  as: Tag = "div",
}: {
  children: React.ReactNode;
  delay?: number;
  className?: string;
  as?: "div" | "section" | "li" | "span";
}) {
  const ref = useRef<HTMLElement>(null);
  const [shown, setShown] = useState(false);

  useEffect(() => {
    const el = ref.current;
    if (!el) return;
    const reduce = window.matchMedia("(prefers-reduced-motion: reduce)");
    if (reduce.matches) {
      setShown(true);
      return;
    }
    const io = new IntersectionObserver(
      ([entry]) => {
        if (entry.isIntersecting) {
          setShown(true);
          io.disconnect();
        }
      },
      { rootMargin: "0px 0px -12% 0px", threshold: 0.1 },
    );
    io.observe(el);
    return () => io.disconnect();
  }, []);

  return (
    <Tag
      ref={ref as React.Ref<never>}
      data-shown={shown}
      className={`reveal ${className}`}
      style={{ ["--reveal-delay" as string]: `${delay}ms` }}
    >
      {children}
    </Tag>
  );
}

const PLATFORMS = [
  { os: "macOS", icon: AppleIcon, file: ".pkg" },
  { os: "Windows", icon: WindowsIcon, file: ".zip" },
  { os: "Linux", icon: LinuxIcon, file: ".deb" },
];

const STEPS = [
  { name: "CAM 1", secs: 120 },
  { name: "CAM 2", secs: 30 },
  { name: "CAM 3", secs: 60 },
];

export default function Site({ lang }: { lang: Lang }) {
  const t = dict[lang];

  return (
    <div className="min-h-screen">
      <Header t={t} lang={lang} />
      <main>
        <Hero t={t} />
        <Pitch t={t} />
        <Features t={t} />
        <How t={t} />
        <Download t={t} />
      </main>
      <Footer t={t} />
    </div>
  );
}

type T = (typeof dict)["en"];

function Header({ t, lang }: { t: T; lang: Lang }) {
  return (
    <header className="sticky top-0 z-50 border-b border-line bg-paper/80 backdrop-blur">
      <div className="mx-auto flex h-16 max-w-6xl items-center justify-between px-6">
        <a href="#top" className="group flex items-center gap-2.5 font-mono text-sm font-bold tracking-tight">
          <Logo className="h-7 w-7 transition-transform duration-500 group-hover:rotate-180" />
          Oh My DJ
        </a>
        <nav className="hidden items-center gap-8 text-sm text-mute md:flex">
          <a href="#features" className="transition-colors hover:text-ink">{t.nav.features}</a>
          <a href="#how" className="transition-colors hover:text-ink">{t.nav.how}</a>
          <a href="#download" className="transition-colors hover:text-ink">{t.nav.download}</a>
          <a href={REPO_URL} className="transition-colors hover:text-ink">{t.nav.github}</a>
        </nav>
        <div className="flex items-center gap-2">
          <a
            href={`/${otherLang(lang)}`}
            hrefLang={otherLang(lang)}
            aria-label={lang === "en" ? "Cambiar a español" : "Switch to English"}
            className="inline-flex h-9 w-12 items-center justify-center border border-line font-mono text-xs font-medium text-mute transition-colors hover:border-ink hover:text-ink"
          >
            {t.langLabel}
          </a>
          <a
            href="#download"
            className="hidden h-9 items-center border border-ink bg-ink px-4 text-sm font-medium text-paper transition-[transform,box-shadow] duration-200 hover:-translate-y-0.5 hover:shadow-[3px_3px_0_0_var(--color-ink)] sm:inline-flex"
          >
            {t.hero.download}
          </a>
        </div>
      </div>
    </header>
  );
}

function Hero({ t }: { t: T }) {
  return (
    <section id="top" className="relative overflow-hidden border-b border-line">
      <div className="grid-bg pointer-events-none absolute inset-0 h-full" aria-hidden />
      <div className="relative mx-auto max-w-6xl px-6 py-20 md:py-28">
        <Reveal as="span" className="inline-flex items-center gap-2 border border-line px-3 py-1 font-mono text-xs uppercase tracking-widest text-mute">
          <span className="h-1.5 w-1.5 animate-pulse rounded-full bg-ink" />
          {t.hero.badge}
        </Reveal>
        <Reveal delay={60}>
          <h1 className="mt-8 text-4xl font-bold leading-[1.02] tracking-tight sm:text-5xl md:text-6xl">
            {t.hero.title}
          </h1>
        </Reveal>
        <Reveal delay={120}>
          <p className="mt-3 text-2xl font-semibold leading-[1.1] text-ink/70 sm:text-3xl md:text-4xl">
            {t.hero.titleAccent}
          </p>
        </Reveal>
        <div className="mt-12 grid items-center gap-12 lg:grid-cols-[1.1fr_0.9fr] lg:gap-16">
          <Reveal delay={200}>
            <p className="max-w-xl text-lg leading-relaxed text-mute">{t.hero.subtitle}</p>
            <div className="mt-10 flex flex-wrap items-center gap-3">
              <a
                href="#download"
                className="group inline-flex h-12 items-center gap-2 border border-ink bg-ink px-6 text-sm font-medium text-paper transition-[transform,box-shadow] duration-200 hover:-translate-y-0.5 hover:shadow-[4px_4px_0_0_var(--color-ink)]"
              >
                {t.hero.download}
                <span aria-hidden className="transition-transform duration-200 group-hover:translate-y-0.5">↓</span>
              </a>
              <a
                href={REPO_URL}
                className="inline-flex h-12 items-center border border-ink px-6 text-sm font-medium transition-colors duration-200 hover:bg-ink hover:text-paper"
              >
                {t.hero.source}
              </a>
            </div>
            <p className="mt-8 max-w-xl font-mono text-xs leading-relaxed text-mute">{t.hero.replaces}</p>
          </Reveal>
          <Reveal delay={280} className="flex justify-center lg:justify-end">
            <DockMock t={t} />
          </Reveal>
        </div>
      </div>
    </section>
  );
}

function DockMock({ t }: { t: T }) {
  const [i, setI] = useState(0);
  const [p, setP] = useState(0);
  const [tab, setTab] = useState<"cameras" | "streaming">("cameras");

  useEffect(() => {
    const reduce = window.matchMedia("(prefers-reduced-motion: reduce)");
    if (reduce.matches) {
      setP(0.45);
      return;
    }
    let raf = 0;
    let start: number | null = null;
    let idx = 0;
    const durationMs = (secs: number) => 1600 + secs * 52;
    const tick = (ts: number) => {
      if (start === null) start = ts;
      const prog = (ts - start) / durationMs(STEPS[idx].secs);
      if (prog >= 1) {
        start = ts;
        idx = (idx + 1) % STEPS.length;
        setI(idx);
        setP(0);
      } else {
        setP(prog);
      }
      raf = requestAnimationFrame(tick);
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, []);

  const cur = STEPS[i];
  const next = STEPS[(i + 1) % STEPS.length];
  const remain = Math.ceil(cur.secs * (1 - p));
  const clock = `${String(Math.floor(remain / 60)).padStart(2, "0")}:${String(
    remain % 60,
  ).padStart(2, "0")}`;

  return (
    <div className="w-full max-w-md border border-ink bg-paper">
      <div className="flex items-center justify-between border-b border-line px-4 py-3">
        <span className="font-mono text-xs font-bold tracking-tight">{t.dock.title}</span>
        <span className="inline-flex items-center gap-1.5 font-mono text-[11px] font-medium">
          <span className="h-1.5 w-1.5 animate-pulse rounded-full bg-ink" />
          {t.dock.live}
        </span>
      </div>
      <div className="flex border-b border-line font-mono text-xs">
        <button
          onClick={() => setTab("cameras")}
          className={`border-r border-line px-4 py-2 transition-colors ${
            tab === "cameras" ? "bg-ink text-paper" : "text-mute hover:text-ink"
          }`}
        >
          {t.dock.cameras}
        </button>
        <button
          onClick={() => setTab("streaming")}
          className={`border-r border-line px-4 py-2 transition-colors ${
            tab === "streaming" ? "bg-ink text-paper" : "text-mute hover:text-ink"
          }`}
        >
          {t.dock.streaming}
        </button>
      </div>
      {tab === "cameras" ? (
        <div className="space-y-3 p-4">
          <div className="flex items-baseline justify-between font-mono text-xs text-mute">
            <span>
              {t.dock.showing}: <span className="font-bold text-ink">{cur.name}</span>
            </span>
            <span className="tabular-nums">
              {clock} → {next.name}
            </span>
          </div>
          <div className="h-1.5 w-full overflow-hidden bg-fog">
            <div className="h-full bg-ink" style={{ width: `${p * 100}%` }} />
          </div>
          <div className="grid grid-cols-3 gap-1.5">
            {STEPS.map((s, idx) => {
              const active = idx === i;
              return (
                <div
                  key={s.name}
                  className={`relative aspect-video overflow-hidden font-mono transition-colors duration-300 ${
                    active ? "bg-ink text-paper" : "bg-fog text-mute"
                  }`}
                >
                  <CamScene index={idx} active={active} />
                  <Corners active={active} />
                  {active && (
                    <span className="absolute right-1.5 top-1.5 h-1.5 w-1.5 animate-pulse rounded-full bg-paper" />
                  )}
                  <div
                    className={`absolute inset-x-0 bottom-0 flex items-center justify-between px-2 py-1 ${
                      active ? "bg-paper/15" : "bg-ink/[0.04]"
                    }`}
                  >
                    <span className="text-[11px] font-bold tracking-tight">{s.name}</span>
                    <span className="text-[10px] tabular-nums opacity-70">{fmtDuration(s.secs)}</span>
                  </div>
                </div>
              );
            })}
          </div>
          <div className="flex items-center gap-2 pt-1 font-mono text-[11px] text-mute">
            <span className="h-1.5 w-1.5 rounded-full bg-ink" />
            {t.dock.sync}
          </div>
        </div>
      ) : (
        <div className="space-y-3 p-4">
          <div className="grid grid-cols-[1fr_auto_auto] items-center gap-x-4 gap-y-3 font-mono text-xs">
            {t.multistream.rows.map((r, idx) => (
              <div key={idx} className="contents">
                <span className="font-bold text-ink">{r[0]}</span>
                <Toggle on={r[2] === "on"} />
                <StatusDot status={r[3]} />
              </div>
            ))}
          </div>
          <div className="flex items-center justify-between border-t border-line pt-3 font-mono text-[11px] text-mute">
            <span className="inline-flex items-center gap-2">
              <span className="h-1.5 w-1.5 rounded-full bg-ink" />
              {t.dock.sync}
            </span>
            <Toggle on={true} />
          </div>
        </div>
      )}
    </div>
  );
}

function fmtDuration(secs: number) {
  return `${Math.floor(secs / 60)}:${String(secs % 60).padStart(2, "0")}`;
}

function CamScene({ index, active }: { index: number; active: boolean }) {
  // Clean, filled glyphs sitting in the upper area so the bottom label bar stays
  // clear. Low-opacity monochrome so they read as a faint camera feed.
  const tone = active ? "rgba(255,255,255,0.20)" : "rgba(10,10,10,0.10)";
  return (
    <svg
      viewBox="0 0 48 27"
      className="pointer-events-none absolute inset-0 h-full w-full"
      aria-hidden
    >
      {index === 0 && (
        // Wide shot — a DJ bust
        <g fill={tone}>
          <circle cx="24" cy="8" r="3.4" />
          <path d="M15.5 19c0-4.7 3.8-8 8.5-8s8.5 3.3 8.5 8z" />
        </g>
      )}
      {index === 1 && (
        // Crowd — a row of heads
        <g fill={tone}>
          <circle cx="14" cy="12.5" r="2.4" />
          <circle cx="20.7" cy="12.5" r="2.4" />
          <circle cx="27.3" cy="12.5" r="2.4" />
          <circle cx="34" cy="12.5" r="2.4" />
        </g>
      )}
      {index === 2 && (
        // Close-up — a lens / vinyl ring
        <g>
          <circle cx="24" cy="11.5" r="6.5" fill="none" stroke={tone} strokeWidth="2.4" />
          <circle cx="24" cy="11.5" r="1.7" fill={tone} />
        </g>
      )}
    </svg>
  );
}

function Corners({ active }: { active: boolean }) {
  const c = active ? "border-paper/40" : "border-ink/15";
  return (
    <>
      <span className={`pointer-events-none absolute left-1 top-1 h-1.5 w-1.5 border-l border-t ${c}`} />
      <span className={`pointer-events-none absolute right-1 top-1 h-1.5 w-1.5 border-r border-t ${c}`} />
      <span className={`pointer-events-none absolute bottom-1 left-1 h-1.5 w-1.5 border-b border-l ${c}`} />
      <span className={`pointer-events-none absolute bottom-1 right-1 h-1.5 w-1.5 border-b border-r ${c}`} />
    </>
  );
}

function Pitch({ t }: { t: T }) {
  return (
    <section className="border-b border-line">
      <div className="mx-auto max-w-6xl px-6 py-16 md:py-24">
        <Reveal>
          <Eyebrow>{t.pitch.tag}</Eyebrow>
          <h2 className="mt-6 max-w-3xl text-3xl font-bold leading-tight tracking-tight md:text-5xl">
            {t.pitch.title}
          </h2>
          <p className="mt-6 max-w-2xl text-lg leading-relaxed text-mute">{t.pitch.body}</p>
        </Reveal>
      </div>
    </section>
  );
}

function Features({ t }: { t: T }) {
  return (
    <section id="features" className="border-b border-line">
      <div className="mx-auto max-w-6xl px-6 py-16 md:py-24">
        <Reveal>
          <Eyebrow>{t.nav.features}</Eyebrow>
          <h2 className="mt-6 max-w-2xl text-3xl font-bold leading-tight tracking-tight md:text-4xl">
            {t.featuresTitle}
          </h2>
        </Reveal>
        <div className="mt-12 grid gap-12 md:grid-cols-2 md:gap-16">
          <Reveal delay={80}>
            <FeatureCard f={t.rotation} kind="rotation" />
          </Reveal>
          <Reveal delay={160}>
            <FeatureCard f={t.multistream} kind="stream" />
          </Reveal>
        </div>
      </div>
    </section>
  );
}

function FeatureCard({
  f,
  kind,
}: {
  f: T["rotation"];
  kind: "rotation" | "stream";
}) {
  return (
    <div className="flex h-full flex-col">
      <span className="font-mono text-xs uppercase tracking-widest text-mute">{f.tag}</span>
      <h3 className="mt-3 text-2xl font-bold tracking-tight">{f.title}</h3>
      <p className="mt-4 text-sm leading-relaxed text-mute">{f.body}</p>
      <div className="mt-auto pt-8 font-mono text-xs">
        <div
          className="grid border-b border-ink/80 pb-2"
          style={{ gridTemplateColumns: cols(kind) }}
        >
          {f.cols.map((c) => (
            <span key={c} className="pr-3 text-[11px] uppercase tracking-wider text-mute">
              {c}
            </span>
          ))}
        </div>
        {f.rows.map((row, i) => (
          <div
            key={i}
            className="grid border-b border-line py-2.5 last:border-0"
            style={{ gridTemplateColumns: cols(kind) }}
          >
            {row.map((cell, j) => (
              <span key={j} className="truncate pr-3 tabular-nums text-mute">
                {kind === "stream" && j === 2 ? <Toggle on={cell === "on"} /> : null}
                {kind === "stream" && j === 3 ? <StatusDot status={cell} /> : null}
                {(kind === "rotation" || j < 2) ? cell : null}
              </span>
            ))}
          </div>
        ))}
      </div>
    </div>
  );
}

function cols(kind: "rotation" | "stream") {
  return kind === "rotation" ? "1fr 0.7fr 1fr 1.4fr" : "1fr 2fr 0.7fr 0.9fr";
}

function Toggle({ on }: { on: boolean }) {
  return (
    <span className={`inline-grid h-4 w-4 place-items-center border ${on ? "border-ink bg-ink text-paper" : "border-line text-transparent"}`}>
      ✓
    </span>
  );
}

function StatusDot({ status }: { status: string }) {
  const live = status === "live";
  return (
    <span className="inline-flex items-center gap-1.5 text-ink">
      <span className={`h-1.5 w-1.5 rounded-full ${live ? "animate-pulse bg-ink" : "bg-line"}`} />
      {live ? "LIVE" : "idle"}
    </span>
  );
}

function How({ t }: { t: T }) {
  return (
    <section id="how" className="border-b border-line">
      <div className="mx-auto max-w-6xl px-6 py-16 md:py-24">
        <Reveal>
          <Eyebrow>{t.howTag}</Eyebrow>
          <h2 className="mt-6 max-w-2xl text-3xl font-bold leading-tight tracking-tight md:text-4xl">
            {t.howTitle}
          </h2>
        </Reveal>
        <div className="mt-12 grid gap-10 md:grid-cols-3 md:gap-12">
          {t.steps.map((s, idx) => (
            <Reveal key={s.n} delay={idx * 90} className="border-t border-ink/80 pt-5">
              <span className="font-mono text-sm font-bold tracking-tight text-mute">{s.n}</span>
              <h3 className="mt-4 text-lg font-bold tracking-tight">{s.title}</h3>
              <p className="mt-3 text-sm leading-relaxed text-mute">{s.body}</p>
            </Reveal>
          ))}
        </div>
      </div>
    </section>
  );
}

type ReleaseAsset = { url: string; count: number };
type ReleaseInfo = { assets: Record<string, ReleaseAsset>; total: number };

function useLatestRelease(): ReleaseInfo | null {
  const [info, setInfo] = useState<ReleaseInfo | null>(null);
  useEffect(() => {
    let active = true;
    fetch("https://api.github.com/repos/vigosan/oh-my-dj/releases?per_page=100", {
      headers: { Accept: "application/vnd.github+json" },
    })
      .then((r) => (r.ok ? r.json() : null))
      .then((releases) => {
        if (!active || !Array.isArray(releases) || releases.length === 0) return;
        const latest = releases.find(
          (rel: { draft?: boolean; prerelease?: boolean }) => !rel.draft && !rel.prerelease,
        );
        if (!latest?.assets) return;
        const find = (test: (n: string) => boolean) =>
          latest.assets.find((a: { name: string }) => test(a.name));
        const assets: Record<string, ReleaseAsset> = {};
        const add = (
          os: string,
          a?: { browser_download_url: string; download_count: number },
        ) => {
          if (a) assets[os] = { url: a.browser_download_url, count: a.download_count };
        };
        add("macOS", find((n) => n.endsWith(".pkg")));
        add("Windows", find((n) => n.toLowerCase().includes("windows") && n.endsWith(".zip")));
        add("Linux", find((n) => n.endsWith(".deb")));
        const total = releases.reduce(
          (sum: number, rel: { assets?: { name: string; download_count: number }[] }) =>
            sum +
            (rel.assets ?? []).reduce(
              (s: number, a: { name: string; download_count: number }) =>
                a.name.endsWith(".ddeb") ? s : s + a.download_count,
              0,
            ),
          0,
        );
        setInfo({ assets, total });
      })
      .catch(() => {});
    return () => {
      active = false;
    };
  }, []);
  return info;
}

function Download({ t }: { t: T }) {
  const release = useLatestRelease();
  return (
    <section id="download" className="border-b border-line">
      <div className="mx-auto max-w-6xl px-6 py-16 md:py-24">
        <Reveal>
          <Eyebrow>{t.download.tag}</Eyebrow>
          <h2 className="mt-6 text-3xl font-bold leading-tight tracking-tight md:text-5xl">
            {t.download.title}
          </h2>
          <p className="mt-6 max-w-2xl text-lg leading-relaxed text-mute">{t.download.body}</p>
          {release && release.total > 0 && (
            <p className="mt-4 font-mono text-xs text-mute">
              ↓ {release.total.toLocaleString()} {t.download.downloads}
            </p>
          )}
        </Reveal>

        <div className="mt-12 grid gap-4 sm:grid-cols-3">
          {PLATFORMS.map(({ os, icon: Icon, file }, idx) => {
            const asset = release?.assets[os];
            return (
              <Reveal key={os} delay={idx * 80}>
                <a
                  href={asset?.url ?? RELEASES_URL}
                  className="group flex h-full flex-col items-center gap-4 bg-fog px-6 py-12 text-center transition-[transform,background-color,color] duration-200 hover:-translate-y-1 hover:bg-ink hover:text-paper"
                >
                  <Icon className="h-10 w-10 transition-transform duration-200 group-hover:scale-110" />
                  <span className="text-lg font-medium tracking-tight">{os}</span>
                  <span className="inline-flex items-center gap-1.5 font-mono text-xs text-mute group-hover:text-paper/70">
                    {file}
                    <span aria-hidden className="transition-transform duration-200 group-hover:translate-y-0.5">↓</span>
                    {asset && <span className="opacity-70">· {asset.count.toLocaleString()}</span>}
                  </span>
                </a>
              </Reveal>
            );
          })}
        </div>
        <p className="mt-6 max-w-2xl font-mono text-xs leading-relaxed text-mute">{t.download.note}</p>
      </div>
    </section>
  );
}

function Footer({ t }: { t: T }) {
  return (
    <footer className="mx-auto max-w-6xl px-6 py-14">
      <div className="flex flex-col items-start justify-between gap-8 md:flex-row md:items-center">
        <div>
          <div className="flex items-center gap-2.5 font-mono text-sm font-bold tracking-tight">
            <Logo className="h-6 w-6" />
            Oh My DJ
          </div>
          <p className="mt-3 max-w-xs text-sm text-mute">{t.footer.tagline}</p>
        </div>
        <div className="flex flex-wrap items-center gap-6 font-mono text-xs text-mute">
          <a href={REPO_URL} className="transition-colors hover:text-ink">GitHub</a>
          <a href={RELEASES_URL} className="transition-colors hover:text-ink">Releases</a>
          <span>{t.footer.license}</span>
        </div>
      </div>
      <div className="mt-10 border-t border-line pt-6 font-mono text-xs text-mute">
        {t.footer.built}
      </div>
    </footer>
  );
}

function Eyebrow({ children }: { children: React.ReactNode }) {
  return (
    <span className="inline-flex items-center gap-2 font-mono text-xs uppercase tracking-widest text-mute">
      <span className="h-px w-8 bg-ink" />
      {children}
    </span>
  );
}

function Logo({ className }: { className?: string }) {
  return (
    <svg viewBox="0 0 32 32" className={className} fill="none" aria-hidden>
      <circle cx="16" cy="16" r="15.5" fill="currentColor" />
      <circle cx="16" cy="16" r="10" stroke="white" strokeOpacity="0.25" />
      <circle cx="16" cy="16" r="6.5" stroke="white" strokeOpacity="0.25" />
      <circle cx="16" cy="16" r="3.4" fill="white" />
      <circle cx="16" cy="16" r="0.9" fill="currentColor" />
    </svg>
  );
}

function AppleIcon({ className }: { className?: string }) {
  return (
    <svg viewBox="0 0 24 24" className={className} fill="currentColor" aria-hidden>
      <path d="M16.36 1.43c0 1.13-.41 2.2-1.11 2.98-.85.97-2.23 1.72-3.34 1.63-.13-1.1.42-2.27 1.05-3 .73-.86 2.04-1.5 3.13-1.55.01.31.27.62.27.94zM20.5 17.2c-.55 1.27-.82 1.84-1.53 2.96-.99 1.56-2.39 3.5-4.12 3.51-1.54.02-1.93-1-4.02-.99-2.09.01-2.52.99-4.06.97-1.73-.02-3.05-1.77-4.04-3.33C-.07 16.92-.4 11.96 1.42 9.4 2.47 7.91 4.12 7.04 5.67 7.04c1.58 0 2.57 1.06 3.88 1.06 1.27 0 2.04-1.06 3.87-1.06 1.38 0 2.84.75 3.88 2.05-3.41 1.87-2.85 6.74 1.32 8.11z" />
    </svg>
  );
}

function WindowsIcon({ className }: { className?: string }) {
  return (
    <svg viewBox="0 0 24 24" className={className} fill="currentColor" aria-hidden>
      <path d="M3 4.5l7.5-1.03v7.28H3zM11.4 3.34L21 2v8.75h-9.6zM3 11.65h7.5v7.28L3 17.9zM11.4 11.65H21V20l-9.6-1.34z" />
    </svg>
  );
}

function LinuxIcon({ className }: { className?: string }) {
  return (
    <svg viewBox="0 0 24 24" className={className} fill="currentColor" aria-hidden>
      <ellipse cx="12" cy="14.5" rx="5.2" ry="6.5" />
      <circle cx="12" cy="6" r="3.6" />
      <ellipse cx="9.6" cy="21" rx="2.1" ry="1.1" />
      <ellipse cx="14.4" cy="21" rx="2.1" ry="1.1" />
    </svg>
  );
}
