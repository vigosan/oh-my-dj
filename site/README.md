# Oh My DJ — website

Marketing/landing site for the [Oh My DJ](https://github.com/vigosan/oh-my-dj)
OBS plugin. Single page, bilingual (EN/ES toggle), tech-minimalist monochrome
design. Built with Next.js 16 + Tailwind v4.

## Local development

```bash
cd site
pnpm install
pnpm dev      # http://localhost:3000
pnpm build    # production build
```

## Deploy to Vercel

The site lives in the `site/` subdirectory of the plugin repo, so the Vercel
project must point at it:

1. Import the `vigosan/oh-my-dj` repo in Vercel.
2. Set **Root Directory** to `site`.
3. Framework preset is auto-detected as **Next.js**. Leave build/output as
   defaults.
4. Deploy.

Or from the CLI, inside this folder:

```bash
cd site
vercel        # preview
vercel --prod # production
```

## Content

All copy lives in `lib/i18n.ts` (one `Dict` per language). Edit there to change
text in both languages; the page reads from the active language.
