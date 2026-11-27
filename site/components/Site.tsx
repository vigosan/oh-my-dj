"use client";

import { useEffect, useState } from "react";
import { dict, type Lang, REPO_URL, RELEASES_URL } from "@/lib/i18n";

const otherLang = (l: Lang): Lang => (l === "en" ? "es" : "en");

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

const ITEM = "border-l-2 border-line pl-4 hover:border-line-strong sm:pl-6";

export default function Site({ lang }: { lang: Lang }) {
  const t = dict[lang];

  return (
    <div
      id="top"
      className="mx-auto flex min-h-screen w-full max-w-5xl flex-col gap-6 px-6 py-12 text-muted antialiased lg:gap-24 lg:px-24 lg:py-16"
    >
      <Header t={t} lang={lang} />
      <main className="flex flex-col gap-8 lg:gap-12">
        <Hero t={t} />
        <Pitch t={t} />
        <Features t={t} />
        <How t={t} />
        <Download t={t} />
        <Faq t={t} />
      </main>
      <Footer t={t} />
    </div>
  );
}

type T = (typeof dict)["en"];

function Header({ t, lang }: { t: T; lang: Lang }) {
  return (
    <header className="flex flex-col gap-4">
      <div className="flex items-start justify-between gap-4">
        <div className="flex flex-col">
          <a href="#top" className="link self-start">
            <span className="font-medium text-ink">Oh My DJ</span>
          </a>
          <span className="text-muted">{t.hero.badge}</span>
        </div>
        <a
          href={`/${otherLang(lang)}`}
          hrefLang={otherLang(lang)}
          aria-label={lang === "en" ? "Cambiar a español" : "Switch to English"}
          className="link px-1 font-medium underline"
        >
          {t.langLabel}
        </a>
      </div>
      <nav className="flex flex-wrap gap-x-4 gap-y-1 text-sm">
        <a href="#features" className="link px-1 underline">{t.nav.features}</a>
        <a href="#how" className="link px-1 underline">{t.nav.how}</a>
        <a href="#faq" className="link px-1 underline">{t.nav.faq}</a>
        <a href="#download" className="link px-1 underline">{t.nav.download}</a>
        <a href={REPO_URL} className="link px-1 underline">
          {t.nav.github} <span aria-hidden>↗</span>
        </a>
      </nav>
    </header>
  );
}

function Heading({ title, tag }: { title: string; tag: string }) {
  return (
    <div>
      <h2 className="text-lg font-medium text-ink">{title}</h2>
      <span className="text-sm text-faint">{tag}</span>
    </div>
  );
}

function Hero({ t }: { t: T }) {
  return (
    <section className="flex flex-col gap-6">
      <div>
        <h1 className="text-lg font-medium text-ink">{t.hero.title}</h1>
        <p className="text-body">{t.hero.titleAccent}</p>
      </div>
      <p className="text-body">{t.hero.subtitle}</p>
      <p className="flex flex-wrap items-baseline gap-x-6 gap-y-2">
        <span>
          <span aria-hidden className="mr-3 text-accent">→</span>
          <a href="#download" className="link px-1 font-medium underline">
            {t.hero.download}
          </a>
        </span>
        <a href={REPO_URL} className="link px-1 underline">
          {t.hero.source} <span aria-hidden>↗</span>
        </a>
      </p>
      <p className="text-sm text-faint">{t.hero.replaces}</p>
      <DockMock t={t} />
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
    <div className="w-full max-w-md border border-line">
      <div className="flex items-center justify-between border-b border-line px-4 py-3 text-xs">
        <span className="font-medium text-ink">{t.dock.title}</span>
        <span className="inline-flex items-center gap-1.5 font-medium text-ink">
          <span className="h-1.5 w-1.5 rounded-full bg-accent" />
          {t.dock.live}
        </span>
      </div>
      <div className="flex border-b border-line text-xs">
        <button
          onClick={() => setTab("cameras")}
          className={`link border-r border-line px-4 py-2 ${
            tab === "cameras" ? "bg-wash font-medium text-ink" : "text-muted"
          }`}
        >
          {t.dock.cameras}
        </button>
        <button
          onClick={() => setTab("streaming")}
          className={`link border-r border-line px-4 py-2 ${
            tab === "streaming" ? "bg-wash font-medium text-ink" : "text-muted"
          }`}
        >
          {t.dock.streaming}
        </button>
      </div>
      {tab === "cameras" ? (
        <div className="space-y-3 p-4">
          <div className="flex items-baseline justify-between text-xs text-muted">
            <span>
              {t.dock.showing}: <span className="font-medium text-ink">{cur.name}</span>
            </span>
            <span className="tabular-nums">
              {clock} → {next.name}
            </span>
          </div>
          <div className="h-1 w-full overflow-hidden bg-wash">
            <div className="h-full bg-ink" style={{ width: `${p * 100}%` }} />
          </div>
          <div className="grid grid-cols-3 gap-1.5">
            {STEPS.map((s, idx) => {
              const active = idx === i;
              return (
                <div
                  key={s.name}
                  className={`relative aspect-video overflow-hidden border-l-2 bg-wash ${
                    active ? "border-accent text-ink" : "border-transparent text-muted"
                  }`}
                >
                  <CamScene index={idx} />
                  <Corners />
                  <div className="absolute inset-x-0 bottom-0 flex items-center justify-between px-2 py-1">
                    <span className="text-[11px] font-medium">{s.name}</span>
                    <span className="text-[10px] tabular-nums text-faint">{fmtDuration(s.secs)}</span>
                  </div>
                </div>
              );
            })}
          </div>
          <div className="flex items-center gap-2 pt-1 text-[11px] text-muted">
            <span className="h-1.5 w-1.5 rounded-full bg-muted" />
            {t.dock.sync}
          </div>
        </div>
      ) : (
        <div className="space-y-3 p-4">
          <div className="grid grid-cols-[1fr_auto_auto] items-center gap-x-4 gap-y-3 text-xs">
            {t.multistream.rows.map((r, idx) => (
              <div key={idx} className="contents">
                <span className="font-medium text-ink">{r[0]}</span>
                <Toggle on={r[2] === "on"} />
                <StatusDot status={r[3]} />
              </div>
            ))}
          </div>
          <div className="flex items-center justify-between border-t border-line pt-3 text-[11px] text-muted">
            <span className="inline-flex items-center gap-2">
              <span className="h-1.5 w-1.5 rounded-full bg-muted" />
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

function CamScene({ index }: { index: number }) {
  return (
    <svg
      viewBox="0 0 48 27"
      className="pointer-events-none absolute inset-0 h-full w-full opacity-20"
      aria-hidden
    >
      {index === 0 && (
        <g fill="currentColor">
          <circle cx="24" cy="8" r="3.4" />
          <path d="M15.5 19c0-4.7 3.8-8 8.5-8s8.5 3.3 8.5 8z" />
        </g>
      )}
      {index === 1 && (
        <g fill="currentColor">
          <circle cx="14" cy="12.5" r="2.4" />
          <circle cx="20.7" cy="12.5" r="2.4" />
          <circle cx="27.3" cy="12.5" r="2.4" />
          <circle cx="34" cy="12.5" r="2.4" />
        </g>
      )}
      {index === 2 && (
        <g>
          <circle cx="24" cy="11.5" r="6.5" fill="none" stroke="currentColor" strokeWidth="2.4" />
          <circle cx="24" cy="11.5" r="1.7" fill="currentColor" />
        </g>
      )}
    </svg>
  );
}

function Corners() {
  const c = "border-line-strong/40";
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
    <section className="flex flex-col gap-6">
      <Heading title={t.pitch.title} tag={t.pitch.tag} />
      <p className="text-body">{t.pitch.body}</p>
    </section>
  );
}

function Features({ t }: { t: T }) {
  return (
    <section id="features" className="flex flex-col gap-6">
      <Heading title={t.featuresTitle} tag={t.nav.features} />
      <div className="flex flex-col gap-8">
        <FeatureCard f={t.rotation} kind="rotation" />
        <FeatureCard f={t.multistream} kind="stream" />
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
    <div className={ITEM}>
      <div className="mb-2">
        <h3 className="font-medium text-ink">{f.title}</h3>
        <span className="text-sm text-faint">{f.tag}</span>
      </div>
      <p className="text-body">{f.body}</p>
      <div className="mt-6 text-xs">
        <div
          className="grid border-b border-line-strong pb-2"
          style={{ gridTemplateColumns: cols(kind) }}
        >
          {f.cols.map((c) => (
            <span key={c} className="pr-3 text-faint">
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
              <span key={j} className="truncate pr-3 tabular-nums text-muted">
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
    <span className={`inline-grid h-4 w-4 place-items-center border ${on ? "border-line-strong text-ink" : "border-line text-transparent"}`}>
      ✓
    </span>
  );
}

function StatusDot({ status }: { status: string }) {
  const live = status === "live";
  return (
    <span className="inline-flex items-center gap-1.5 text-ink">
      <span className={`h-1.5 w-1.5 rounded-full ${live ? "bg-accent" : "bg-line"}`} />
      {live ? "LIVE" : "idle"}
    </span>
  );
}

function How({ t }: { t: T }) {
  return (
    <section id="how" className="flex flex-col gap-6">
      <Heading title={t.howTitle} tag={t.howTag} />
      <div className="flex flex-col gap-4">
        {t.steps.map((s) => (
          <div key={s.n} className={ITEM}>
            <div className="mb-2 flex gap-3">
              <span className="text-faint">{s.n}</span>
              <h3 className="font-medium text-ink">{s.title}</h3>
            </div>
            <p className="text-body">{s.body}</p>
          </div>
        ))}
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
    <section id="download" className="flex flex-col gap-6">
      <Heading title={t.download.title} tag={t.download.tag} />
      <p className="text-body">{t.download.body}</p>
      {release && release.total > 0 && (
        <p className="text-sm text-faint">
          ↓ {release.total.toLocaleString()} {t.download.downloads}
        </p>
      )}
      <div className="flex flex-col gap-4">
        {PLATFORMS.map(({ os, icon: Icon, file }) => {
          const asset = release?.assets[os];
          return (
            <div key={os} className={ITEM}>
              <a
                href={asset?.url ?? RELEASES_URL}
                className="link inline-flex items-center gap-2 px-1 font-medium underline"
              >
                <Icon className="h-4 w-4 text-muted" />
                {os}
              </a>
              <span className="ml-3 text-sm text-faint">
                {file} <span aria-hidden>↓</span>
                {asset && <> · {asset.count.toLocaleString()}</>}
              </span>
            </div>
          );
        })}
      </div>
      <p className="text-sm text-faint">{t.download.note}</p>
    </section>
  );
}

function Faq({ t }: { t: T }) {
  return (
    <section id="faq" className="flex flex-col gap-6">
      <Heading title={t.faqTitle} tag={t.faqTag} />
      <div className="flex flex-col gap-4">
        {t.faq.map((item, idx) => (
          <details key={idx} className={`group ${ITEM}`}>
            <summary className="link flex cursor-pointer list-none items-start justify-between gap-4 px-1 font-medium text-ink">
              {item.q}
              <span
                aria-hidden
                className="text-muted transition-transform duration-200 ease-out group-open:rotate-45 motion-reduce:transition-none"
              >
                +
              </span>
            </summary>
            <p className="mt-2 px-1 text-body">{item.a}</p>
          </details>
        ))}
      </div>
    </section>
  );
}

function Footer({ t }: { t: T }) {
  return (
    <footer className="flex flex-col gap-2 border-t border-line pt-6 text-sm text-faint">
      <p>
        <span className="font-medium text-ink">Oh My DJ</span>
        <span className="block text-muted">{t.footer.tagline}</span>
      </p>
      <p className="flex flex-wrap items-baseline gap-x-4 gap-y-1">
        <a href={REPO_URL} className="link px-1 underline">
          GitHub <span aria-hidden>↗</span>
        </a>
        <a href={RELEASES_URL} className="link px-1 underline">
          Releases <span aria-hidden>↗</span>
        </a>
        <span>{t.footer.license}</span>
      </p>
      <p className="flex flex-wrap items-baseline gap-x-4 gap-y-1">
        <span>{t.footer.built}</span>
        <a href="https://vicent.io" className="link px-1 underline">
          by Vicent Gozalbes <span aria-hidden>↗</span>
        </a>
      </p>
    </footer>
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
