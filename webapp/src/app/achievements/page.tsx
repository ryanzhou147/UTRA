"use client";

const achievements = [
  { id: "curved_ramp", name: "CURVED_RAMP_CLEARED", description: "Complete the obstacle course", unlocked: true, teams: ["ROBO-042", "MECH-017", "BOLT-099"] },
  { id: "blue_zone", name: "BLUE_ZONE_SHOT", description: "Hit the blue zone in target shooting", unlocked: true, teams: ["ROBO-042", "TITAN-003"] },
  { id: "under_60", name: "UNDER_60_SECONDS", description: "Complete obstacle course in under 60s", unlocked: true, teams: ["MECH-017"] },
  { id: "zero_resets", name: "ZERO_RESETS", description: "Complete without using any resets", unlocked: true, teams: ["BOLT-099"] },
  { id: "perfect_run", name: "PERFECT_RUN", description: "No penalties and no resets", unlocked: false, teams: [] },
  { id: "sharpshooter", name: "SHARPSHOOTER", description: "Hit 3+ rings in one run", unlocked: true, teams: ["ROBO-042", "MECH-017", "NOVA-021"] },
  { id: "consistency", name: "CONSISTENCY_KING", description: "< 5% variance across 3+ runs", unlocked: false, teams: [] },
  { id: "improved", name: "MOST_IMPROVED", description: "Score increased 20%+ from first run", unlocked: true, teams: ["NOVA-021"] },
];

export default function AchievementsPage() {
  const unlocked = achievements.filter(a => a.unlocked).length;

  return (
    <div className="max-w-5xl mx-auto">
      <div className="mb-6 text-[#6c7086] text-sm">{">"} ACHIEVEMENTS [{unlocked}/{achievements.length}]</div>

      <div className="grid md:grid-cols-2 gap-4">
        {achievements.map((a) => (
          <div key={a.id} className={`terminal-box p-5 transition-all ${a.unlocked ? "" : "opacity-50"}`}>
            <div className="flex items-start justify-between mb-2">
              <div className="text-[#cdd6f4] font-medium">
                {a.unlocked ? <span className="text-[#a6e3a1] mr-2">[X]</span> : <span className="text-[#6c7086] mr-2">[ ]</span>}
                {a.name}
              </div>
              {a.unlocked && <span className="text-[#fab387]">*</span>}
            </div>
            <div className="text-[#6c7086] text-sm mb-3">{a.description}</div>
            {a.teams.length > 0 && (
              <div className="flex flex-wrap gap-2">
                {a.teams.map(t => (
                  <span key={t} className="text-xs px-2 py-1 bg-[#313145] text-[#89b4fa] rounded">{t}</span>
                ))}
              </div>
            )}
          </div>
        ))}
      </div>

      <div className="mt-6 text-xs text-[#6c7086]">{">"} Unlocked badges can be minted as NFTs on Solana</div>
    </div>
  );
}
