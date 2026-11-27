import { NextRequest, NextResponse } from "next/server";
import { LANG_COOKIE, pickLang } from "./lib/lang";

export function proxy(req: NextRequest) {
  const lang = pickLang(req.cookies.get(LANG_COOKIE)?.value, req.headers.get("accept-language"));
  const url = req.nextUrl.clone();
  url.pathname = `/${lang}`;
  return NextResponse.redirect(url);
}

export const config = {
  matcher: "/",
};
