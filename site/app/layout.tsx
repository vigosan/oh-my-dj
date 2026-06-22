import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Oh My DJ — OBS plugin for DJs",
  description:
    "An OBS plugin for DJs: one dock to rotate cameras on a timer and multistream to several platforms. Automatic, lightweight, translated.",
  metadataBase: new URL("https://oh-my-dj.vercel.app"),
  openGraph: {
    title: "Oh My DJ — OBS plugin for DJs",
    description:
      "One dock to rotate cameras on a timer and multistream to several platforms. Automatic and lightweight.",
    type: "website",
  },
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en">
      <body className="font-sans antialiased">{children}</body>
    </html>
  );
}
