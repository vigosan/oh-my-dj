import { renderToStaticMarkup } from "react-dom/server";
import { describe, expect, it } from "vitest";
import Site from "./Site";
import { LANGS, RELEASES_URL, REPO_URL } from "@/lib/i18n";

describe.each(LANGS)("site in %s", (lang) => {
  const html = renderToStaticMarkup(<Site lang={lang} />);

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

  it("renders every section visible from the first paint, so content never waits for a scroll to appear", () => {
    expect(html).not.toContain('data-shown="false"');
  });

  it("links to the source code for people who want to build it themselves", () => {
    expect(html).toContain(`href="${REPO_URL}"`);
  });
});
