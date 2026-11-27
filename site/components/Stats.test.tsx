import { renderToStaticMarkup } from "react-dom/server";
import { afterEach, describe, expect, it, vi } from "vitest";
import { dict, LANGS } from "@/lib/i18n";
import type { Stats as StatsData } from "@/lib/stats";
import { Page } from "./Page";
import { Stats, StatsView } from "./Stats";

afterEach(() => vi.unstubAllGlobals());

const stats: StatsData = {
  days: [
    { date: "2026-10-25", mac: 5, windows: 4, linux: 0, estimated: true },
    { date: "2026-11-01", mac: 22, windows: 11, linux: 1 },
  ],
  versions: [
    { version: "0.7.0", mac: 7, windows: 9, linux: 2 },
    { version: "0.6.2", mac: 2, windows: 1, linux: 0 },
  ],
};

const text = (html: string, testid: string) => html.match(new RegExp(`data-testid="${testid}"[^>]*>([^<]*)<`))?.[1];

const view = (s: StatsData, lang: "en" | "es" = "es") => renderToStaticMarkup(<StatsView lang={lang} stats={s} />);

describe("stats page", () => {
  it("heads with the downloads of every system on the latest day", () => {
    expect(text(view(stats), "stats-total")).toBe("34");
  });

  it("shows each system apart, so a visitor sees where the plugin is used", () => {
    const html = view(stats);
    expect(text(html, "stats-mac")).toBe("22");
    expect(text(html, "stats-windows")).toBe("11");
    expect(text(html, "stats-linux")).toBe("1");
  });

  it("groups thousands, since Spanish leaves four-digit numbers ungrouped and they read as a year", () => {
    const big = { ...stats, days: [{ date: "2026-11-01", mac: 1200, windows: 0, linux: 0 }] };
    expect(text(view(big), "stats-total")).toBe("1.200");
  });

  it("lists each version newest first with its downloads per system", () => {
    const html = view(stats);
    expect(html.indexOf("0.7.0")).toBeLessThan(html.indexOf("0.6.2"));
    expect(text(html, "stats-version-windows-0.7.0")).toBe("9");
    expect(text(html, "stats-version-linux-0.7.0")).toBe("2");
  });

  it("says from which day the history stops being a reconstruction, in each language, so nobody reads the estimate as measured", () => {
    expect(text(view(stats), "stats-measured-since")).toContain("1 de noviembre");
    expect(text(view(stats, "en"), "stats-measured-since")).toContain("November 1");
  });

  it("explains that the figures could not be loaded instead of showing an empty page when GitHub fails", async () => {
    vi.spyOn(console, "error").mockImplementation(() => {});
    vi.stubGlobal("fetch", async () => new Response("", { status: 500 }));
    for (const lang of LANGS) {
      const html = renderToStaticMarkup(await Stats({ lang }));
      expect(html).toContain('data-testid="stats-error"');
      expect(html).toContain(dict[lang].stats.error);
    }
  });

  it("links the stats page from the footer of every page in both languages", () => {
    for (const lang of LANGS) {
      const html = renderToStaticMarkup(
        <Page lang={lang} current="home">
          {null}
        </Page>,
      );
      const footer = html.slice(html.indexOf("<footer"));
      expect(footer, lang).toContain(`href="/${lang}/stats"`);
      expect(footer, lang).toContain(dict[lang].footer.stats);
    }
  });
});
