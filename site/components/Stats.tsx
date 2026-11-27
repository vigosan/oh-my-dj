import { dict, type Lang } from "@/lib/i18n";
import { fetchStats, installsInLast, type Stats as StatsData, totalInstalls, type Week, weeklyInstalls } from "@/lib/stats";
import { Page } from "./Page";

const heading = "text-2xl font-semibold tracking-tight text-balance text-gray-900 dark:text-gray-50";
const axis = "fill-gray-400 font-mono text-[22px] sm:text-[11px] dark:fill-gray-500";
const grid = "stroke-gray-200 dark:stroke-gray-800";

const W = 720;
const LEFT = 52;
const RIGHT = 12;
const TOP = 12;
const BOTTOM = 36;

const PLATFORMS = [
  { key: "mac", fill: "fill-gray-800 dark:fill-gray-100", dot: "bg-gray-800 dark:bg-gray-100" },
  { key: "windows", fill: "fill-gray-800/45 dark:fill-gray-100/45", dot: "bg-gray-800/45 dark:bg-gray-100/45" },
  { key: "linux", fill: "fill-gray-800/20 dark:fill-gray-100/20", dot: "bg-gray-800/20 dark:bg-gray-100/20" },
] as const;

function niceMax(value: number): number {
  const step = 10 ** Math.floor(Math.log10(Math.max(value, 1)));
  const nice = [1, 2, 2.5, 5, 10].map((m) => m * step).find((n) => n * 4 >= value) ?? step * 10;
  return nice * 4;
}

const ticks = (max: number) => [0, 1, 2, 3, 4].map((i) => (max / 4) * i);

export function formatter(lang: Lang) {
  const number = new Intl.NumberFormat(lang, { useGrouping: "always" } as unknown as Intl.NumberFormatOptions);
  return {
    number: (n: number) => number.format(n),
    month: (date: string) => new Date(date).toLocaleDateString(lang, { month: "short", timeZone: "UTC" }),
    day: (date: string) => new Date(date).toLocaleDateString(lang, { day: "numeric", month: "long", timeZone: "UTC" }),
  };
}

type Format = ReturnType<typeof formatter>;

function Grid({ max, y, format }: { max: number; y: (v: number) => number; format: Format }) {
  return ticks(max).map((v) => (
    <g key={v}>
      <line x1={LEFT} x2={W - RIGHT} y1={y(v)} y2={y(v)} className={grid} />
      <text x={LEFT - 8} y={y(v) + 4} textAnchor="end" className={axis}>
        {format.number(v)}
      </text>
    </g>
  ));
}

function WeeklyBars({ weeks, label, format }: { weeks: Week[]; label: string; format: Format }) {
  const H = 180;
  const max = niceMax(Math.max(...weeks.map((w) => w.mac + w.windows + w.linux)));
  const band = (W - LEFT - RIGHT) / weeks.length;
  const y = (v: number) => TOP + (1 - v / max) * (H - TOP - BOTTOM);

  return (
    <svg viewBox={`0 0 ${W} ${H}`} className="h-auto w-full overflow-visible" role="img" aria-label={label}>
      <Grid max={max} y={y} format={format} />
      {weeks.map((w, i) => {
        let below = 0;
        return (
          <g key={w.week}>
            {PLATFORMS.map(({ key, fill }) => {
              const from = below;
              below += w[key];
              return (
                <rect key={key} x={LEFT + i * band + band * 0.15} y={y(below)} width={band * 0.7} height={y(from) - y(below)} className={fill}>
                  <title>{`${format.day(w.week)}: ${format.number(w[key])}`}</title>
                </rect>
              );
            })}
            {(i === 0 || w.week.slice(0, 7) !== weeks[i - 1].week.slice(0, 7)) && (
              <text x={LEFT + i * band + band / 2} y={H - 6} textAnchor="middle" className={axis}>
                {format.month(w.week)}
              </text>
            )}
          </g>
        );
      })}
    </svg>
  );
}

function Legend({ lang }: { lang: Lang }) {
  const t = dict[lang].stats;
  return (
    <p className="flex gap-5 text-xs text-gray-500">
      {PLATFORMS.map(({ key, dot }) => (
        <span key={key} className="flex items-center gap-1.5">
          <span className={`size-2 rounded-full ${dot}`} />
          {t[key]}
        </span>
      ))}
    </p>
  );
}

function Figure({ value, label, testid }: { value: string; label: string; testid: string }) {
  return (
    <div className="flex flex-col gap-0.5 border-t border-gray-200 pt-3 dark:border-gray-800">
      <span data-testid={testid} className="text-2xl font-semibold tabular-nums text-gray-900 dark:text-gray-50">
        {value}
      </span>
      <span className="text-sm">{label}</span>
    </div>
  );
}

export function StatsView({ lang, stats }: { lang: Lang; stats: StatsData }) {
  const t = dict[lang].stats;
  const format = formatter(lang);
  const days = stats.days;
  const latest = days[days.length - 1];
  const measuredSince = days.find((d) => !d.estimated)?.date ?? latest.date;
  return (
    <>
      <section className="flex flex-col gap-3">
        <p data-testid="stats-total" className="text-6xl font-semibold tracking-tight tabular-nums text-gray-900 sm:text-7xl dark:text-gray-50">
          {format.number(totalInstalls(stats))}
        </p>
        <p>{t.total.replace("{date}", format.day(days[0].date))}</p>
        <div className="mt-5 grid gap-4 sm:grid-cols-3">
          {PLATFORMS.map(({ key }) => (
            <Figure key={key} value={format.number(latest[key])} label={t[key]} testid={`stats-${key}`} />
          ))}
        </div>
        <div className="mt-5 flex flex-wrap gap-x-8 gap-y-2 text-sm">
          <span>
            <b className="font-mono text-xl font-semibold tabular-nums text-gray-900 dark:text-gray-50">+{format.number(installsInLast(days, 7))}</b> {t.lastWeek}
          </span>
          <span>
            <b className="font-mono text-xl font-semibold tabular-nums text-gray-900 dark:text-gray-50">+{format.number(installsInLast(days, 30))}</b> {t.lastMonth}
          </span>
        </div>
      </section>

      <article data-testid="stats-weekly" className="flex flex-col gap-6">
        <h2 className={heading}>{t.weeklyTitle}</h2>
        <WeeklyBars weeks={weeklyInstalls(days)} label={t.weeklyTitle} format={format} />
        <Legend lang={lang} />
      </article>

      <article data-testid="stats-versions" className="flex flex-col gap-6">
        <h2 className={heading}>{t.versionsTitle}</h2>
        <p className="max-w-2xl text-pretty">{t.versionsText}</p>
        <table className="w-full text-sm tabular-nums">
          <thead>
            <tr className="text-xs text-gray-500">
              <td />
              {PLATFORMS.map(({ key }) => (
                <th key={key} className="pb-2 pl-3 text-right font-medium">
                  {t[key]}
                </th>
              ))}
            </tr>
          </thead>
          <tbody>
            {stats.versions.map((v) => (
              <tr key={v.version} className="border-t border-gray-200 dark:border-gray-800">
                <th className="py-2 text-left font-mono text-xs font-normal text-gray-800 dark:text-gray-100">{v.version}</th>
                {PLATFORMS.map(({ key }) => (
                  <td key={key} data-testid={`stats-version-${key}-${v.version}`} className="py-2 pl-3 text-right font-mono">
                    {format.number(v[key])}
                  </td>
                ))}
              </tr>
            ))}
          </tbody>
        </table>
      </article>

      <article className="flex flex-col gap-6">
        <h2 className={heading}>{t.howTitle}</h2>
        <ul className="flex flex-col gap-3 leading-relaxed">
          {[...t.how, t.measuredSince.replace("{date}", format.day(measuredSince))].map((item, index) => (
            <li key={item} className="sm:grid sm:grid-cols-[2.5rem_1fr]">
              <span className="hidden font-mono text-xs leading-7 tabular-nums text-gray-400 sm:inline dark:text-gray-600">{String(index + 1).padStart(2, "0")}</span>
              <span data-testid={index === t.how.length ? "stats-measured-since" : undefined}>{item}</span>
            </li>
          ))}
        </ul>
      </article>
    </>
  );
}

export async function Stats({ lang }: { lang: Lang }) {
  const t = dict[lang].stats;
  const stats = await fetchStats();

  return (
    <Page lang={lang} current="stats">
      <section className="flex flex-col gap-5">
        <h1 className="max-w-2xl text-4xl font-semibold tracking-tight text-balance text-gray-900 sm:text-5xl dark:text-gray-50">{t.title}</h1>
        <p className="max-w-xl text-lg text-pretty">{t.lede}</p>
      </section>
      {stats ? <StatsView lang={lang} stats={stats} /> : <p data-testid="stats-error">{t.error}</p>}
    </Page>
  );
}
