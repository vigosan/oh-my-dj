"use client";

import type { Lang } from "@/lib/i18n";
import { langCookie } from "@/lib/lang";

export function LangSwitch({ lang, href, label, className, children }: { lang: Lang; href: string; label: string; className: string; children: React.ReactNode }) {
  return (
    <a href={href} hrefLang={lang} aria-label={label} className={className} onClick={() => (document.cookie = langCookie(lang))}>
      {children}
    </a>
  );
}
