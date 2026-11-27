import type { MetadataRoute } from "next";
import { LANGS, SITE_URL } from "@/lib/i18n";

export default function sitemap(): MetadataRoute.Sitemap {
  const home = LANGS.map((lang) => ({
    url: `${SITE_URL}/${lang}`,
    changeFrequency: "monthly" as const,
    priority: lang === "en" ? 1 : 0.9,
    alternates: {
      languages: {
        en: `${SITE_URL}/en`,
        es: `${SITE_URL}/es`,
      },
    },
  }));
  const stats = LANGS.map((lang) => ({
    url: `${SITE_URL}/${lang}/stats`,
    changeFrequency: "daily" as const,
    priority: 0.5,
    alternates: {
      languages: {
        en: `${SITE_URL}/en/stats`,
        es: `${SITE_URL}/es/stats`,
      },
    },
  }));
  return [...home, ...stats];
}
