import type { Metadata } from "next";
import { GeistMono } from "geist/font/mono";
import { GeistSans } from "geist/font/sans";
import "../globals.css";
import { LANGS, SITE_URL, type Lang } from "@/lib/i18n";
import { THEME_SCRIPT } from "@/lib/theme";

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
    <html lang={htmlLang} suppressHydrationWarning>
      <head>
        <script dangerouslySetInnerHTML={{ __html: THEME_SCRIPT }} />
      </head>
      <body className={`${GeistSans.variable} ${GeistMono.variable} font-sans antialiased`}>{children}</body>
    </html>
  );
}
