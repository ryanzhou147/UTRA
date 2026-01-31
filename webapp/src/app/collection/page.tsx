"use client";

import { useState } from "react";

interface NFT {
  id: string;
  name: string;
  description: string;
  imageUrl: string;
  team: string;
  achievement: string;
  mintedAt: string;
  rarity: "common" | "rare" | "epic" | "legendary";
}

const mockNFTs: NFT[] = [
  {
    id: "nft-001",
    name: "Blue Zone Master",
    description: "Awarded for hitting the blue zone 3 times in a row",
    imageUrl: "https://picsum.photos/seed/nft1/400/400",
    team: "ROBO-042",
    achievement: "BLUE_ZONE_SHOT",
    mintedAt: "2024-01-31 14:32",
    rarity: "rare",
  },
  {
    id: "nft-002",
    name: "Speed Demon",
    description: "Completed obstacle course in under 45 seconds",
    imageUrl: "https://picsum.photos/seed/nft2/400/400",
    team: "MECH-017",
    achievement: "UNDER_60_SECONDS",
    mintedAt: "2024-01-31 13:15",
    rarity: "epic",
  },
  {
    id: "nft-003",
    name: "Precision Champion",
    description: "Achieved 95%+ consistency across 5 runs",
    imageUrl: "https://picsum.photos/seed/nft3/400/400",
    team: "BOLT-099",
    achievement: "CONSISTENCY_KING",
    mintedAt: "2024-01-31 12:00",
    rarity: "legendary",
  },
  {
    id: "nft-004",
    name: "First Blood",
    description: "First team to complete the obstacle course",
    imageUrl: "https://picsum.photos/seed/nft4/400/400",
    team: "ROBO-042",
    achievement: "CURVED_RAMP_CLEARED",
    mintedAt: "2024-01-31 10:30",
    rarity: "common",
  },
  {
    id: "nft-005",
    name: "Sharpshooter Elite",
    description: "Hit 5 rings in a single target shooting run",
    imageUrl: "https://picsum.photos/seed/nft5/400/400",
    team: "TITAN-003",
    achievement: "SHARPSHOOTER",
    mintedAt: "2024-01-31 09:45",
    rarity: "rare",
  },
  {
    id: "nft-006",
    name: "Perfect Run",
    description: "Completed obstacle course with zero penalties",
    imageUrl: "https://picsum.photos/seed/nft6/400/400",
    team: "MECH-017",
    achievement: "PERFECT_RUN",
    mintedAt: "2024-01-31 15:20",
    rarity: "legendary",
  },
  {
    id: "nft-007",
    name: "Zero Resets",
    description: "Completed without using any resets",
    imageUrl: "https://picsum.photos/seed/nft7/400/400",
    team: "BOLT-099",
    achievement: "ZERO_RESETS",
    mintedAt: "2024-01-30 16:10",
    rarity: "epic",
  },
  {
    id: "nft-008",
    name: "Rising Star",
    description: "Improved score by 50% from first to last run",
    imageUrl: "https://picsum.photos/seed/nft8/400/400",
    team: "NOVA-021",
    achievement: "MOST_IMPROVED",
    mintedAt: "2024-01-30 14:55",
    rarity: "rare",
  },
];

const rarityColors = {
  common: "#a6adc8",
  rare: "#89b4fa",
  epic: "#cba6f7",
  legendary: "#fab387",
};

export default function CollectionPage() {
  const [filter, setFilter] = useState<"all" | "common" | "rare" | "epic" | "legendary">("all");
  const [selected, setSelected] = useState<NFT | null>(null);

  const filteredNFTs = filter === "all" ? mockNFTs : mockNFTs.filter((n) => n.rarity === filter);

  const stats = {
    total: mockNFTs.length,
    common: mockNFTs.filter((n) => n.rarity === "common").length,
    rare: mockNFTs.filter((n) => n.rarity === "rare").length,
    epic: mockNFTs.filter((n) => n.rarity === "epic").length,
    legendary: mockNFTs.filter((n) => n.rarity === "legendary").length,
  };

  return (
    <div className="max-w-7xl mx-auto">
      <div className="mb-6 text-[#6c7086] text-sm">{">"} NFT COLLECTION</div>

      {/* Stats Bar */}
      <div className="terminal-box p-4 mb-6">
        <div className="flex flex-wrap gap-6 text-sm">
          <div className="text-[#cdd6f4]">
            <span className="text-[#6c7086]">Total:</span> {stats.total}
          </div>
          <div style={{ color: rarityColors.common }}>
            <span className="text-[#6c7086]">Common:</span> {stats.common}
          </div>
          <div style={{ color: rarityColors.rare }}>
            <span className="text-[#6c7086]">Rare:</span> {stats.rare}
          </div>
          <div style={{ color: rarityColors.epic }}>
            <span className="text-[#6c7086]">Epic:</span> {stats.epic}
          </div>
          <div style={{ color: rarityColors.legendary }}>
            <span className="text-[#6c7086]">Legendary:</span> {stats.legendary}
          </div>
        </div>
      </div>

      {/* Filter Tabs */}
      <div className="flex gap-2 mb-6">
        {(["all", "common", "rare", "epic", "legendary"] as const).map((f) => (
          <button
            key={f}
            onClick={() => setFilter(f)}
            className={`px-4 py-2 text-sm rounded transition-colors ${
              filter === f
                ? "bg-[#313145] border border-[#cba6f7] text-[#cba6f7]"
                : "bg-[#282839] border border-[#45475a] text-[#6c7086] hover:text-[#a6adc8]"
            }`}
          >
            {f.toUpperCase()}
          </button>
        ))}
      </div>

      <div className="grid lg:grid-cols-3 gap-6">
        {/* NFT Grid */}
        <div className="lg:col-span-2">
          <div className="grid sm:grid-cols-2 md:grid-cols-3 gap-4">
            {filteredNFTs.map((nft) => (
              <div
                key={nft.id}
                onClick={() => setSelected(nft)}
                className={`terminal-box p-3 cursor-pointer transition-all hover:scale-[1.02] ${
                  selected?.id === nft.id ? "ring-2 ring-[#cba6f7]" : ""
                }`}
              >
                <div className="aspect-square bg-[#1e1e2e] rounded-lg mb-3 overflow-hidden">
                  <img
                    src={nft.imageUrl}
                    alt={nft.name}
                    className="w-full h-full object-cover"
                  />
                </div>
                <div className="text-[#cdd6f4] text-sm font-medium truncate">{nft.name}</div>
                <div className="flex justify-between items-center mt-2">
                  <span className="text-[#6c7086] text-xs">{nft.team}</span>
                  <span
                    className="text-xs px-2 py-0.5 rounded"
                    style={{
                      color: rarityColors[nft.rarity],
                      backgroundColor: `${rarityColors[nft.rarity]}20`,
                    }}
                  >
                    {nft.rarity}
                  </span>
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Detail Panel */}
        <div className="lg:col-span-1">
          <div className="terminal-box p-5 sticky top-6">
            {selected ? (
              <>
                <div className="aspect-square bg-[#1e1e2e] rounded-lg mb-4 overflow-hidden">
                  <img
                    src={selected.imageUrl}
                    alt={selected.name}
                    className="w-full h-full object-cover"
                  />
                </div>
                <div className="text-[#cba6f7] text-lg font-medium mb-2">{selected.name}</div>
                <div className="text-[#a6adc8] text-sm mb-4">{selected.description}</div>

                <div className="space-y-3 text-sm">
                  <div className="flex justify-between">
                    <span className="text-[#6c7086]">Team</span>
                    <span className="text-[#cdd6f4]">{selected.team}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[#6c7086]">Achievement</span>
                    <span className="text-[#89b4fa]">{selected.achievement}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[#6c7086]">Rarity</span>
                    <span style={{ color: rarityColors[selected.rarity] }}>
                      {selected.rarity.toUpperCase()}
                    </span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[#6c7086]">Minted</span>
                    <span className="text-[#a6adc8]">{selected.mintedAt}</span>
                  </div>
                </div>

                <div className="mt-6 pt-4 border-t border-[#45475a]">
                  <div className="text-xs text-[#6c7086] mb-2">NFT ID</div>
                  <div className="text-xs text-[#a6adc8] font-mono break-all">
                    {selected.id}
                  </div>
                </div>
              </>
            ) : (
              <div className="h-64 flex items-center justify-center text-[#6c7086]">
                SELECT AN NFT
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}
