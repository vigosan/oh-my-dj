import { renderToStaticMarkup } from "react-dom/server";
import { describe, expect, it } from "vitest";
import Site from "./Site";
import { dict, LANGS, RELEASES_URL, REPO_URL } from "@/lib/i18n";

describe.each(LANGS)("site in %s", (lang) => {
  const html = renderToStaticMarkup(<Site lang={lang} stats={null} />);

  it("keeps the in-page navigation so visitors can jump straight to what they came for", () => {
    for (const id of ["features", "how", "faq", "download"]) {
      expect(html).toContain(`href="#${id}"`);
      expect(html).toContain(`id="${id}"`);
    }
  });

  it("offers the other language so a visitor who landed in the wrong one can switch", () => {
    const other = lang === "en" ? "es" : "en";
    expect(html).toContain(`href="/${other}"`);
    expect(html).toContain(`hrefLang="${other}"`);
  });

  it("points every download at the releases page until the latest assets are known, so no button is ever dead", () => {
    const downloads = html.match(new RegExp(`href="${RELEASES_URL}"`, "g")) ?? [];
    expect(downloads.length).toBeGreaterThanOrEqual(3);
  });

  it("links to the source code for people who want to build it themselves", () => {
    expect(html).toContain(`href="${REPO_URL}"`);
  });
});

describe.each(LANGS)("theme in %s", (lang) => {
  const html = renderToStaticMarkup(<Site lang={lang} stats={null} />);

  it("offers a theme button in the header that starts on system, so the page follows the visitor's Mac or PC until they pick otherwise", () => {
    const header = html.slice(html.indexOf("<header"), html.indexOf("</header>"));
    expect(header).toContain(`data-testid="theme-toggle"`);
    expect(header).toContain(`aria-label="${dict[lang].theme.system}"`);
  });
});

describe.each(LANGS)("family layout in %s", (lang) => {
  const t = dict[lang];
  const html = renderToStaticMarkup(<Site lang={lang} stats={null} />);
  const header = html.slice(html.indexOf("<header"), html.indexOf("</header>"));
  const footer = html.slice(html.indexOf("<footer"), html.indexOf("</footer>"));

  it("opens the header with the logo and the name linking home, like every sibling product site", () => {
    expect(header).toMatch(new RegExp(`<a[^>]*data-testid="home-link"[^>]*href="/${lang}"[^>]*><svg[^>]*>.*</svg><span[^>]*>Oh My DJ</span></a>`));
  });

  it("leads the hero with the headline, then the intro, then the download, then the live dock, so the promise is read before the demo", () => {
    const order = [t.hero.title, t.hero.subtitle, 'data-testid="download"', 'data-testid="dock"'].map((s) => html.indexOf(s));
    expect(order.every((i) => i > -1)).toBe(true);
    expect([...order].sort((a, b) => a - b)).toEqual(order);
  });

  it("sends the hero download to the download section, which is also where the plugin's update notice lands", () => {
    expect(html).toMatch(/data-testid="download"[^>]*href="#download"|href="#download"[^>]*data-testid="download"/);
  });

  it("keeps both dock tabs clickable, so visitors can see the cameras and the streaming side of the plugin", () => {
    expect(html).toMatch(/<button[^>]*data-testid="dock-tab-cameras"/);
    expect(html).toMatch(/<button[^>]*data-testid="dock-tab-streaming"/);
  });

  it("keeps every FAQ answer one tap away, since they double as the page's search snippets", () => {
    expect(html.match(/<details/g)).toHaveLength(t.faq.length);
  });

  it("credits the author in the footer with a link back to vicent.io, like the other product sites", () => {
    expect(footer).toContain(t.footer.by);
    expect(footer).toContain('href="https://vicent.io"');
    expect(footer).toContain(`href="${REPO_URL}"`);
    expect(footer).toContain(`href="${RELEASES_URL}"`);
  });
});
