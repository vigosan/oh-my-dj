import type { Metadata } from "next";
import { notFound } from "next/navigation";
import { Stats } from "@/components/Stats";
import { dict, LANGS, SITE_URL, type Lang } from "@/lib/i18n";

function isLang(value: string): value is Lang {
  return (LANGS as string[]).includes(value);
}

export async function generateMetadata({ params }: { params: Promise<{ lang: string }> }): Promise<Metadata> {
  const { lang } = await params;
  if (!isLang(lang)) return {};
  const title = `${dict[lang].stats.title} · Oh My DJ`;
  return {
    title,
    description: dict[lang].stats.lede,
    alternates: {
      canonical: `${SITE_URL}/${lang}/stats`,
      languages: { en: `${SITE_URL}/en/stats`, es: `${SITE_URL}/es/stats` },
    },
    openGraph: { title, description: dict[lang].stats.lede, url: `${SITE_URL}/${lang}/stats`, siteName: "Oh My DJ", type: "website" },
  };
}

export default async function StatsPage({ params }: { params: Promise<{ lang: string }> }) {
  const { lang } = await params;
  if (!isLang(lang)) notFound();
  return <Stats lang={lang} />;
}
