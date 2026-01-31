import type { Metadata } from "next";
import { JetBrains_Mono } from "next/font/google";
import "./globals.css";
import Link from "next/link";
import ClientLayout from "@/components/ClientLayout";

const mono = JetBrains_Mono({ subsets: ["latin"] });

export const metadata: Metadata = {
  title: "UTRA Terminal",
  description: "Robot telemetry and run tracking",
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en" suppressHydrationWarning>
      <body className={`${mono.className} crt min-h-screen`}>
        <nav className="border-b border-[#45475a] px-6 py-3 flex gap-6 text-sm bg-[#282839]">
          <Link href="/" className="text-[#cba6f7] font-semibold">UTRA</Link>
          <Link href="/" className="text-[#a6adc8] hover:text-[#cdd6f4] transition-colors">[logs]</Link>
          <Link href="/runs" className="text-[#a6adc8] hover:text-[#cdd6f4] transition-colors">[runs]</Link>
          <Link href="/achievements" className="text-[#a6adc8] hover:text-[#cdd6f4] transition-colors">[achievements]</Link>
          <Link href="/collection" className="text-[#a6adc8] hover:text-[#cdd6f4] transition-colors">[collection]</Link>
          <Link href="/mint" className="text-[#a6adc8] hover:text-[#cdd6f4] transition-colors">[mint]</Link>
        </nav>
        <ClientLayout>
          <main className="p-6">{children}</main>
        </ClientLayout>
      </body>
    </html>
  );
}
