import Link from "next/link";
import { dict, type Lang, RELEASES_URL, REPO_URL } from "@/lib/i18n";
import { LangSwitch } from "./LangSwitch";
import { Logo } from "./Logo";
import { ThemeToggle } from "./ThemeToggle";

const focus = "rounded focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-gray-400";

export const link = `${focus} font-medium text-gray-900 underline-offset-4 hover:underline dark:text-gray-100`;

const quiet = `${focus} transition-colors duration-150 hover:text-gray-900 dark:hover:text-gray-100`;

export const base =
  "inline-flex items-center gap-3 rounded-lg px-5 py-2.5 font-medium transition-[background-color,border-color,transform] duration-150 ease-out active:scale-[0.97] focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-gray-400 focus-visible:ring-offset-2 dark:focus-visible:ring-offset-neutral-950";
export const button = `${base} bg-gray-800 text-white hover:bg-gray-700 dark:bg-gray-100 dark:text-gray-900 dark:hover:bg-white`;

const otherLang = (l: Lang): Lang => (l === "en" ? "es" : "en");

export function Page({ lang, current, children }: { lang: Lang; current: "home" | "stats"; children: React.ReactNode }) {
  const t = dict[lang];
  const other = otherLang(lang);
  const anchor = (id: string) => (current === "home" ? `#${id}` : `/${lang}#${id}`);

  return (
    <div className="mx-auto flex min-h-dvh w-full max-w-3xl flex-col gap-10 px-6 py-12 font-sans text-gray-600 antialiased lg:gap-16 lg:py-16 dark:text-gray-400">
      <header className="sticky top-0 z-40 -mx-6 -my-3 flex items-center justify-between bg-white/80 px-6 py-3 backdrop-blur dark:bg-neutral-950/80">
        <Link
          data-testid="home-link"
          href={`/${lang}`}
          className="group flex items-center gap-3 rounded focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-gray-400"
        >
          <Logo className="size-9 text-gray-900 transition-transform duration-500 group-hover:rotate-180 dark:text-gray-50" />
          <span className="font-semibold text-gray-900 dark:text-gray-50">Oh My DJ</span>
        </Link>
        <div className="flex items-center gap-3">
          <nav className="mr-3 hidden items-center gap-5 text-sm md:flex">
            <a href={anchor("features")} className={quiet}>{t.nav.features}</a>
            <a href={anchor("how")} className={quiet}>{t.nav.how}</a>
            <a href={anchor("faq")} className={quiet}>{t.nav.faq}</a>
            <a href={anchor("download")} className={quiet}>{t.nav.download}</a>
            <a href={REPO_URL} className={quiet}>{t.nav.github}</a>
          </nav>
          <ThemeToggle labels={t.theme} />
          <LangSwitch
            lang={other}
            href={`/${other}`}
            label={lang === "en" ? "Cambiar a español" : "Switch to English"}
            className={quiet}
          >
            {t.langLabel}
          </LangSwitch>
        </div>
      </header>
      <main className="flex flex-1 flex-col gap-20 lg:gap-28">{children}</main>
      <footer className="flex flex-col gap-4 text-sm">
        <p className="flex items-center gap-2.5">
          <Logo className="size-5 flex-none text-gray-900 dark:text-gray-50" />
          {t.footer.tagline}
        </p>
        <div className="flex flex-wrap gap-x-4 gap-y-2">
          <a href={REPO_URL} className={quiet}>
            GitHub
          </a>
          <a href={RELEASES_URL} className={quiet}>
            Releases
          </a>
          <span>{t.footer.license}</span>
          <span>{t.footer.built}</span>
          <span className="sm:ml-auto">
            {t.footer.by}{" "}
            <a href="https://vicent.io" className={quiet}>
              Vicent Gozalbes
            </a>
          </span>
        </div>
      </footer>
    </div>
  );
}
