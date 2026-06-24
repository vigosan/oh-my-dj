import type { Metadata } from "next";
import { notFound } from "next/navigation";
import Site from "@/components/Site";
import {
  LANGS,
  SITE_URL,
  REPO_URL,
  meta,
  type Lang,
} from "@/lib/i18n";

function isLang(value: string): value is Lang {
  return (LANGS as string[]).includes(value);
}

export async function generateMetadata({
  params,
}: {
  params: Promise<{ lang: string }>;
}): Promise<Metadata> {
  const { lang } = await params;
  if (!isLang(lang)) return {};
  const m = meta[lang];
  return {
    title: m.title,
    description: m.description,
    keywords: m.keywords,
    alternates: {
      canonical: `${SITE_URL}/${lang}`,
      languages: {
        en: `${SITE_URL}/en`,
        es: `${SITE_URL}/es`,
        "x-default": `${SITE_URL}/en`,
      },
    },
    openGraph: {
      title: m.title,
      description: m.description,
      url: `${SITE_URL}/${lang}`,
      siteName: "Oh My DJ",
      locale: lang === "es" ? "es_ES" : "en_US",
      type: "website",
    },
    twitter: {
      card: "summary_large_image",
      title: m.title,
      description: m.description,
    },
  };
}

function JsonLd({ lang }: { lang: Lang }) {
  const m = meta[lang];
  const data = {
    "@context": "https://schema.org",
    "@type": "SoftwareApplication",
    name: "Oh My DJ",
    description: m.description,
    url: `${SITE_URL}/${lang}`,
    applicationCategory: "MultimediaApplication",
    operatingSystem: "macOS, Windows, Linux",
    inLanguage: lang,
    softwareRequirements: "OBS Studio",
    isAccessibleForFree: true,
    offers: { "@type": "Offer", price: "0", priceCurrency: "USD" },
    keywords: m.keywords.join(", "),
    codeRepository: REPO_URL,
    license: "https://opensource.org/licenses/MIT",
  };
  return (
    <script
      type="application/ld+json"
      dangerouslySetInnerHTML={{ __html: JSON.stringify(data) }}
    />
  );
}

export default async function LangPage({
  params,
}: {
  params: Promise<{ lang: string }>;
}) {
  const { lang } = await params;
  if (!isLang(lang)) notFound();
  return (
    <>
      <JsonLd lang={lang} />
      <Site lang={lang} />
    </>
  );
}
