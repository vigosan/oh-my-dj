import { readFileSync } from "node:fs";
import { describe, expect, it } from "vitest";

const css = readFileSync(new URL("./globals.css", import.meta.url), "utf8");

describe("global styles", () => {
  it("follows the system dark mode so the page doesn't glare at DJs reading it in a dark booth", () => {
    expect(css).toContain("prefers-color-scheme: dark");
  });

  it("only paints hover backgrounds on devices that can hover, so a tap on a phone doesn't leave a link highlighted", () => {
    expect(css).toMatch(/@media \(hover: hover\) and \(pointer: fine\)\s*{\s*\.link:hover/);
  });

  it("shows the focus ring for keyboard users only, so clicking with a mouse doesn't flash an outline", () => {
    expect(css).toContain(".link:focus-visible");
    expect(css).not.toMatch(/\.link:focus\s*{/);
  });

  it("acknowledges a press immediately so links feel like they heard the click", () => {
    expect(css).toContain(".link:active");
  });

  it("sets everything in the monospace face shared with vicent.io instead of a system sans", () => {
    expect(css).toContain("--font-geist-mono");
    expect(css).not.toContain("sans-serif");
  });

  it("tints text selection with the product accent, one of the few places the color is allowed to show", () => {
    expect(css).toMatch(/::selection\s*{[^}]*var\(--color-accent\)/);
  });

  it("stops smooth scrolling for visitors who asked for less motion", () => {
    expect(css).toMatch(/prefers-reduced-motion: reduce[\s\S]*scroll-behavior: auto/);
  });
});
