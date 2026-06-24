import { NextRequest, NextResponse } from "next/server";

export function proxy(req: NextRequest) {
  const accept = req.headers.get("accept-language") ?? "";
  const prefersEs = accept.toLowerCase().split(",").some((part) => part.trim().startsWith("es"));
  const lang = prefersEs ? "es" : "en";
  const url = req.nextUrl.clone();
  url.pathname = `/${lang}`;
  return NextResponse.redirect(url);
}

export const config = {
  matcher: "/",
};
