import { NextRequest } from "next/server";
import { describe, expect, it } from "vitest";
import { proxy } from "./proxy";

const visit = (headers: Record<string, string>) =>
  new URL(proxy(new NextRequest("https://oh-my-dj.vicent.io/", { headers })).headers.get("location") ?? "").pathname;

describe("proxy", () => {
  it("sends a browser that prefers Spanish to the Spanish page", () => {
    expect(visit({ "accept-language": "es-ES,es;q=0.9" })).toBe("/es");
  });

  it("sends everyone else to the English page", () => {
    expect(visit({ "accept-language": "fr-FR" })).toBe("/en");
  });

  it("keeps the language picked with the switch, so a Spanish browser that chose English is not bounced back", () => {
    expect(visit({ "accept-language": "es-ES", cookie: "lang=en" })).toBe("/en");
  });
});
