import { afterEach, describe, expect, it, vi } from "vitest";
import { fetchStats, installsInLast, releaseTotals, snapshot, type StatsRelease, totalInstalls, weeklyInstalls } from "./stats";

afterEach(() => vi.unstubAllGlobals());

const asset = (name: string, download_count: number) => ({ name, download_count });

const release = (tag: string, published_at: string, counts: [number, number, number, number], prerelease = false): StatsRelease => ({
  tag_name: tag,
  published_at,
  prerelease,
  assets: [
    asset(`oh-my-dj-${tag}-macos-universal.pkg`, counts[0]),
    asset(`oh-my-dj-${tag}-windows-x64.zip`, counts[1]),
    asset(`oh-my-dj-${tag}-x86_64-linux-gnu.deb`, counts[2]),
    asset(`oh-my-dj-${tag}-x86_64-linux-gnu-dbgsym.ddeb`, counts[3]),
  ],
});

describe("releaseTotals", () => {
  it("counts the installer of each system and leaves the Linux debug symbols out, since nobody needs them to use the plugin", () => {
    expect(releaseTotals(release("0.7.0", "2026-08-08T12:00:00Z", [7, 9, 2, 1]))).toEqual({ mac: 7, windows: 9, linux: 2 });
  });
});

describe("snapshot", () => {
  const releases = [release("0.1.0", "2026-06-01T00:00:00Z", [10, 4, 0, 3]), release("0.2.0", "2026-06-11T00:00:00Z", [6, 2, 2, 0])];
  const stats = snapshot(releases, "2026-06-21T10:00:00Z");

  it("ends on today's exact totals of every release, the same sum the download section shows", () => {
    expect(stats.days.at(-1)).toEqual({ date: "2026-06-21", mac: 16, windows: 6, linux: 2 });
    expect(totalInstalls(stats)).toBe(24);
  });

  it("marks every earlier day as estimated, since GitHub only gives today's count and the history is rebuilt from the release dates", () => {
    expect(stats.days.slice(0, -1).every((d) => d.estimated)).toBe(true);
    expect(stats.days[0].date).toBe("2026-06-01");
  });

  it("lists the versions newest first and keeps prereleases out, so a test build never reads as a release", () => {
    const withBeta = snapshot([...releases, release("0.3.0-beta", "2026-06-15T00:00:00Z", [5, 5, 5, 0], true)], "2026-06-21T10:00:00Z");
    expect(withBeta.versions.map((v) => v.version)).toEqual(["0.2.0", "0.1.0"]);
    expect(withBeta.versions[0]).toEqual({ version: "0.2.0", mac: 6, windows: 2, linux: 2 });
  });
});

describe("installsInLast and weeklyInstalls", () => {
  const days = [
    { date: "2026-06-01", mac: 0, windows: 0, linux: 0 },
    { date: "2026-06-08", mac: 3, windows: 1, linux: 0 },
    { date: "2026-06-15", mac: 5, windows: 2, linux: 1 },
  ];

  it("counts the downloads of every system in the last days", () => {
    expect(installsInLast(days, 7)).toBe(4);
  });

  it("splits the history into weeks starting on Monday", () => {
    expect(weeklyInstalls(days).map((w) => w.mac + w.windows + w.linux)).toEqual([0, 4, 4]);
  });
});

describe("fetchStats", () => {
  it("builds the stats from the GitHub releases of the plugin", async () => {
    vi.stubGlobal("fetch", async () => Response.json([release("0.1.0", "2026-06-01T00:00:00Z", [3, 1, 0, 0])]));
    const stats = await fetchStats();
    expect(stats && totalInstalls(stats)).toBe(4);
  });

  it("gives nothing when GitHub fails or has no releases, so the page hides the figures instead of showing zero", async () => {
    vi.spyOn(console, "error").mockImplementation(() => {});
    vi.stubGlobal("fetch", async () => new Response("rate limited", { status: 403 }));
    expect(await fetchStats()).toBeNull();
    vi.stubGlobal("fetch", async () => Response.json([]));
    expect(await fetchStats()).toBeNull();
    vi.stubGlobal("fetch", async () => {
      throw new Error("offline");
    });
    expect(await fetchStats()).toBeNull();
  });
});
