import { describe, expect, it } from "vitest";
import { SITE_URL } from "@/lib/i18n";
import sitemap from "./sitemap";

describe("sitemap", () => {
  it("lists the stats page in both languages next to the home, so search engines find it", () => {
    const urls = sitemap().map((entry) => entry.url);
    expect(urls).toEqual(expect.arrayContaining([`${SITE_URL}/en`, `${SITE_URL}/es`, `${SITE_URL}/en/stats`, `${SITE_URL}/es/stats`]));
  });
});
