import { renderToStaticMarkup } from "react-dom/server";
import { describe, expect, it } from "vitest";
import { LANGS } from "@/lib/i18n";
import type { Stats } from "@/lib/stats";
import { HeroStats } from "./HeroStats";
import Site from "./Site";

const stats: Stats = {
  days: [
    { date: "2026-11-01", mac: 300, windows: 800, linux: 10 },
    { date: "2026-11-04", mac: 335, windows: 845, linux: 10 },
    { date: "2026-11-11", mac: 371, windows: 928, linux: 12 },
  ],
  versions: [],
};

const render = (s: Stats, lang: "en" | "es" = "es") => renderToStaticMarkup(<HeroStats lang={lang} stats={s} />);

const text = (html: string, testid: string) => html.match(new RegExp(`data-testid="${testid}"[^>]*>([^<]*)<`))?.[1];

describe("hero stats", () => {
  it("shows the same total the stats page heads with, thousands grouped so it never reads as a year", () => {
    expect(text(render(stats), "hero-stats-total")).toBe("1.311");
    expect(text(render(stats, "en"), "hero-stats-total")).toBe("1,311");
  });

  it("counts the downloads of the last seven days", () => {
    expect(text(render(stats), "hero-stats-week")).toBe("+121");
  });

  it("leaves the weekly figure out when the week brought no downloads, since it would read as nobody downloads this next to the button", () => {
    const flat = { ...stats, days: stats.days.map((d) => ({ ...d, mac: 371, windows: 928, linux: 12 })) };
    expect(render(flat)).not.toContain("hero-stats-week");
  });

  it("links the stats page in each language", () => {
    for (const lang of LANGS) expect(render(stats, lang)).toMatch(new RegExp(`data-testid="hero-stats-link" href="/${lang}/stats"`));
  });
});

describe.each(LANGS)("hero in %s", (lang) => {
  it("shows the count under the download and above the dock, so it is seen before scrolling", () => {
    const html = renderToStaticMarkup(<Site lang={lang} stats={stats} />);
    const pill = html.indexOf('data-testid="hero-stats"');
    expect(pill).toBeGreaterThan(html.indexOf('data-testid="download"'));
    expect(pill).toBeLessThan(html.indexOf('data-testid="dock"'));
  });

  it("leaves the count out when the stats cannot be loaded, rather than showing an empty pill", () => {
    expect(renderToStaticMarkup(<Site lang={lang} stats={null} />)).not.toContain('data-testid="hero-stats"');
  });
});
