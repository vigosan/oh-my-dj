export const RELEASES_API = "https://api.github.com/repos/vigosan/oh-my-dj/releases?per_page=100";

export type StatsAsset = { name: string; download_count: number };

export type StatsRelease = {
  tag_name: string;
  published_at: string | null;
  prerelease?: boolean;
  assets?: StatsAsset[];
};

export type Totals = { mac: number; windows: number; linux: number };

export type Day = Totals & { date: string; estimated?: true };

export type Version = Totals & { version: string };

export type Week = Totals & { week: string };

export type Stats = { days: Day[]; versions: Version[] };

const DAY_MS = 86_400_000;

const zero = (): Totals => ({ mac: 0, windows: 0, linux: 0 });

const sumOf = (t: Totals) => t.mac + t.windows + t.linux;

export function releaseTotals(release: StatsRelease): Totals {
  const assets = release.assets ?? [];
  const sum = (test: (name: string) => boolean) => assets.filter((a) => test(a.name)).reduce((total, a) => total + a.download_count, 0);
  return {
    mac: sum((n) => n.endsWith(".pkg")),
    windows: sum((n) => n.toLowerCase().includes("windows") && n.endsWith(".zip")),
    linux: sum((n) => n.endsWith(".deb")),
  };
}

const published = (releases: StatsRelease[]) =>
  releases
    .filter((r): r is StatsRelease & { published_at: string } => r.published_at !== null)
    .sort((a, b) => a.published_at.localeCompare(b.published_at));

export function versions(releases: StatsRelease[], limit: number): Version[] {
  return published(releases)
    .filter((r) => !r.prerelease)
    .map((r) => ({ version: r.tag_name, ...releaseTotals(r) }))
    .reverse()
    .slice(0, limit);
}

const dateOf = (ms: number) => new Date(ms).toISOString().slice(0, 10);

const mondayOf = (date: string) => {
  const ms = Date.parse(date);
  return dateOf(ms - ((new Date(ms).getUTCDay() + 6) % 7) * DAY_MS);
};

const rounded = (date: string, t: Totals): Day => ({ date, mac: Math.round(t.mac), windows: Math.round(t.windows), linux: Math.round(t.linux) });

function rebuild(releases: StatsRelease[], today: string): Day[] {
  const list = published(releases);
  const end = Date.parse(today) + DAY_MS;
  const perDay = new Map<string, Totals>();
  list.forEach((release, i) => {
    const totals = releaseTotals(release);
    const from = Date.parse(release.published_at);
    const to = i + 1 < list.length ? Date.parse(list[i + 1].published_at) : end;
    const span = Math.max(to - from, 1);
    for (let t = from; t < to; ) {
      const next = Math.min(to, Math.floor(t / DAY_MS) * DAY_MS + DAY_MS);
      const share = (next - t) / span;
      const date = dateOf(t);
      const day = perDay.get(date) ?? zero();
      day.mac += totals.mac * share;
      day.windows += totals.windows * share;
      day.linux += totals.linux * share;
      perDay.set(date, day);
      t = next;
    }
  });
  const running = zero();
  return [...perDay.entries()]
    .filter(([date]) => date < today)
    .sort(([a], [b]) => a.localeCompare(b))
    .map(([date, day]) => {
      running.mac += day.mac;
      running.windows += day.windows;
      running.linux += day.linux;
      return { ...rounded(date, running), estimated: true as const };
    });
}

export function snapshot(releases: StatsRelease[], now: string): Stats {
  const today = now.slice(0, 10);
  const totals = published(releases)
    .map(releaseTotals)
    .reduce((sum, t) => ({ mac: sum.mac + t.mac, windows: sum.windows + t.windows, linux: sum.linux + t.linux }), zero());
  return { days: [...rebuild(releases, today), rounded(today, totals)], versions: versions(releases, 8) };
}

export function totalInstalls(stats: Stats): number {
  return sumOf(stats.days[stats.days.length - 1]);
}

export function installsInLast(days: Day[], count: number): number {
  const last = days.at(-1);
  if (!last) return 0;
  const cutoff = dateOf(Date.parse(last.date) - count * DAY_MS);
  const start = days.filter((d) => d.date <= cutoff).at(-1) ?? days[0];
  return sumOf(last) - sumOf(start);
}

export function weeklyInstalls(days: Day[]): Week[] {
  if (days.length === 0) return [];
  const endOfWeek = new Map<string, Totals>();
  for (const d of days) endOfWeek.set(mondayOf(d.date), d);
  const weeks: Week[] = [];
  let before: Totals = zero();
  const last = Date.parse(mondayOf(days[days.length - 1].date));
  for (let ms = Date.parse(mondayOf(days[0].date)); ms <= last; ms += 7 * DAY_MS) {
    const week = dateOf(ms);
    const total = endOfWeek.get(week) ?? before;
    weeks.push({ week, mac: total.mac - before.mac, windows: total.windows - before.windows, linux: total.linux - before.linux });
    before = total;
  }
  return weeks;
}

export async function fetchStats(): Promise<Stats | null> {
  try {
    const response = await fetch(RELEASES_API, { headers: { Accept: "application/vnd.github+json" }, next: { revalidate: 3600 } });
    if (!response.ok) {
      console.error(`Stats request failed: ${response.status} ${await response.text()}`);
      return null;
    }
    const releases = (await response.json()) as StatsRelease[];
    if (!Array.isArray(releases) || published(releases).length === 0) {
      console.error("Stats request returned no releases");
      return null;
    }
    return snapshot(releases, new Date().toISOString());
  } catch (error) {
    console.error(`Stats request failed: ${error}`);
    return null;
  }
}
