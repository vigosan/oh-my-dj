import { ImageResponse } from "next/og";
import { type Lang } from "@/lib/i18n";

export const size = { width: 1200, height: 630 };
export const contentType = "image/png";

export function generateStaticParams() {
  return [{ lang: "en" }, { lang: "es" }];
}

const copy: Record<Lang, { tag: string; title: string; sub: string }> = {
  en: {
    tag: "OBS STUDIO PLUGIN",
    title: "Multistream & multi-camera for DJs",
    sub: "Stream to Twitch, YouTube and Facebook at once. Rotate cameras on a timer. Automatically.",
  },
  es: {
    tag: "PLUGIN DE OBS STUDIO",
    title: "Multistream y multicámara para DJs",
    sub: "Emite a Twitch, YouTube y Facebook a la vez. Rota cámaras con temporizador. Automático.",
  },
};

export default async function Image({
  params,
}: {
  params: Promise<{ lang: string }>;
}) {
  const { lang } = await params;
  const c = copy[lang === "es" ? "es" : "en"];
  return new ImageResponse(
    (
      <div
        style={{
          height: "100%",
          width: "100%",
          display: "flex",
          flexDirection: "column",
          justifyContent: "space-between",
          background: "#ffffff",
          color: "#0a0a0a",
          padding: "72px",
          fontFamily: "sans-serif",
        }}
      >
        <div style={{ display: "flex", alignItems: "center", gap: 20 }}>
          <div
            style={{
              width: 56,
              height: 56,
              borderRadius: 56,
              background: "#0a0a0a",
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
            }}
          >
            <div style={{ width: 18, height: 18, borderRadius: 18, background: "#fff" }} />
          </div>
          <div style={{ fontSize: 34, fontWeight: 700, letterSpacing: -1 }}>Oh My DJ</div>
        </div>
        <div style={{ display: "flex", flexDirection: "column", gap: 24 }}>
          <div
            style={{
              fontSize: 22,
              letterSpacing: 6,
              color: "#6b6b6b",
              fontFamily: "monospace",
            }}
          >
            {c.tag}
          </div>
          <div style={{ fontSize: 76, fontWeight: 800, lineHeight: 1.02, letterSpacing: -2 }}>
            {c.title}
          </div>
          <div style={{ fontSize: 30, color: "#6b6b6b", maxWidth: 980, lineHeight: 1.3 }}>
            {c.sub}
          </div>
        </div>
        <div
          style={{
            display: "flex",
            justifyContent: "space-between",
            alignItems: "center",
            fontSize: 22,
            color: "#6b6b6b",
            fontFamily: "monospace",
            borderTop: "2px solid #e4e4e4",
            paddingTop: 28,
          }}
        >
          <span>oh-my-dj.vicent.io</span>
          <span>macOS · Windows · Linux · Free</span>
        </div>
      </div>
    ),
    { ...size },
  );
}
