"use client";

const achievements = [
  { id: "curved_ramp", name: "CURVED_RAMP_CLEARED", unlocked: true, icon: "[]" },
  { id: "blue_zone", name: "BLUE_ZONE_SHOT", unlocked: true, icon: "[]" },
  { id: "under_60", name: "UNDER_60_SECONDS", unlocked: true, icon: "[]" },
  { id: "zero_resets", name: "ZERO_RESETS", unlocked: false, icon: "[]" },
  { id: "perfect_run", name: "PERFECT_RUN", unlocked: false, icon: "[]" },
  { id: "sharpshooter", name: "SHARPSHOOTER", unlocked: true, icon: "[]" },
];

export default function AchievementsPage() {
  const unlocked = achievements.filter(a => a.unlocked).length;

  return (
    <div className="max-w-2xl mx-auto">
      <div className="mb-6 text-[#6c7086] text-sm">{">"} ACHIEVEMENTS [{unlocked}/{achievements.length}]</div>

      <div className="terminal-box p-5">
        <pre className="text-sm text-[#6c7086]">
{`╔════════════════════════════════════════╗
║           BADGE COLLECTION             ║
╠════════════════════════════════════════╣`}
        </pre>

        {achievements.map((a) => (
          <div
            key={a.id}
            className={`py-3 px-3 border-b border-[#45475a]/50 transition-colors ${
              a.unlocked ? "text-[#cdd6f4]" : "text-[#6c7086]"
            }`}
          >
            {a.unlocked ? (
              <span className="text-[#a6e3a1]">[X]</span>
            ) : (
              <span className="text-[#6c7086]">[ ]</span>
            )}{" "}
            {a.name}
            {a.unlocked && <span className="text-[#fab387] ml-2">*</span>}
          </div>
        ))}

        <pre className="text-sm text-[#6c7086] mt-2">
{`╚════════════════════════════════════════╝`}
        </pre>

        <div className="mt-5 text-xs text-[#6c7086]">
          {">"} Unlocked badges can be minted as NFTs on Solana
        </div>
      </div>
    </div>
  );
}
