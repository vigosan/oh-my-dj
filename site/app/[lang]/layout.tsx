import type { Metadata } from "next";
import "../globals.css";
import { LANGS, SITE_URL, type Lang } from "@/lib/i18n";

export const metadata: Metadata = {
  metadataBase: new URL(SITE_URL),
};

export function generateStaticParams() {
  return LANGS.map((lang) => ({ lang }));
}

export default async function LangLayout({
  children,
  params,
}: {
  children: React.ReactNode;
  params: Promise<{ lang: string }>;
}) {
  const { lang } = await params;
  const htmlLang: Lang = lang === "es" ? "es" : "en";
  return (
    <html lang={htmlLang}>
      <body className="font-sans antialiased">{children}</body>
    </html>
  );
}
