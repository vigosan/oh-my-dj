import type { MetadataRoute } from "next";
import { LANGS, SITE_URL } from "@/lib/i18n";

export default function sitemap(): MetadataRoute.Sitemap {
  return LANGS.map((lang) => ({
    url: `${SITE_URL}/${lang}`,
    changeFrequency: "monthly",
    priority: lang === "en" ? 1 : 0.9,
    alternates: {
      languages: {
        en: `${SITE_URL}/en`,
        es: `${SITE_URL}/es`,
      },
    },
  }));
}
