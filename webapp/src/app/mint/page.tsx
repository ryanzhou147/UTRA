"use client";

import { useState, useCallback } from "react";
import { useWallet } from "@solana/wallet-adapter-react";
import { WalletMultiButton } from "@solana/wallet-adapter-react-ui";
import DragDropZone from "@/components/DragDropZone";

interface MintedNFT {
  id: string;
  name: string;
  imageUrl: string;
  mintAddress: string;
  timestamp: string;
}

export default function MintPage() {
  const { connected, publicKey } = useWallet();
  const [imageFile, setImageFile] = useState<File | null>(null);
  const [imagePreview, setImagePreview] = useState<string | null>(null);
  const [nftName, setNftName] = useState("");
  const [nftDescription, setNftDescription] = useState("");
  const [isMinting, setIsMinting] = useState(false);
  const [mintedNFTs, setMintedNFTs] = useState<MintedNFT[]>([]);
  const [mintStatus, setMintStatus] = useState<string | null>(null);

  const handleImageFile = useCallback((file: File) => {
    setImageFile(file);
    const url = URL.createObjectURL(file);
    setImagePreview(url);
  }, []);

  const handleMint = async () => {
    if (!connected || !publicKey || !imageFile || !nftName) {
      setMintStatus("ERROR: Missing required fields or wallet not connected");
      return;
    }

    setIsMinting(true);
    setMintStatus("INITIALIZING MINT SEQUENCE...");

    // Simulate minting process (in production, this would interact with Metaplex/Solana)
    await new Promise((resolve) => setTimeout(resolve, 1000));
    setMintStatus("UPLOADING IMAGE TO ARWEAVE...");

    await new Promise((resolve) => setTimeout(resolve, 1500));
    setMintStatus("CREATING METADATA...");

    await new Promise((resolve) => setTimeout(resolve, 1000));
    setMintStatus("MINTING NFT ON SOLANA...");

    await new Promise((resolve) => setTimeout(resolve, 2000));

    // Generate mock mint address
    const mockMintAddress = `${publicKey.toString().slice(0, 8)}...${Math.random().toString(36).substring(2, 10)}`;

    const newNFT: MintedNFT = {
      id: `nft-${mintedNFTs.length + 1}`,
      name: nftName,
      imageUrl: imagePreview!,
      mintAddress: mockMintAddress,
      timestamp: new Date().toISOString().slice(0, 16).replace("T", " "),
    };

    setMintedNFTs([newNFT, ...mintedNFTs]);
    setMintStatus("MINT COMPLETE!");
    setIsMinting(false);

    // Reset form
    setTimeout(() => {
      setImageFile(null);
      setImagePreview(null);
      setNftName("");
      setNftDescription("");
      setMintStatus(null);
    }, 2000);
  };

  return (
    <div className="max-w-4xl mx-auto">
      <div className="mb-6 text-[#6c7086] text-sm">{">"} NFT MINT STATION</div>

      <div className="grid md:grid-cols-2 gap-4">
        {/* Mint Form */}
        <div className="terminal-box p-5">
          <div className="text-[#cba6f7] mb-4 font-medium">WALLET STATUS</div>

          <div className="mb-4">
            <WalletMultiButton className="!bg-[#313145] !border !border-[#45475a] !text-[#cdd6f4] hover:!bg-[#45475a] !font-mono !text-sm !h-auto !py-2 !rounded" />
          </div>

          {connected && publicKey && (
            <div className="text-xs text-[#6c7086] mb-4 break-all">
              Connected: {publicKey.toString()}
            </div>
          )}

          <div className="text-[#cba6f7] mb-4 mt-6 font-medium">CREATE NFT</div>

          <div className="space-y-4">
            <div>
              <label className="text-[#6c7086] text-xs block mb-2">IMAGE</label>
              <DragDropZone
                accept={["image/*"]}
                onFile={handleImageFile}
                label="DROP IMAGE FILE"
                sublabel="or click to browse (.png, .jpg, .gif)"
                preview={imagePreview}
                className="h-48"
              />
            </div>

            <div>
              <label className="text-[#6c7086] text-xs block mb-2">NAME</label>
              <input
                type="text"
                value={nftName}
                onChange={(e) => setNftName(e.target.value)}
                className="w-full bg-[#1e1e2e] border border-[#45475a] p-2 text-[#cdd6f4] text-sm rounded focus:border-[#cba6f7] outline-none transition-colors"
                placeholder="UTRA Achievement #1"
              />
            </div>

            <div>
              <label className="text-[#6c7086] text-xs block mb-2">DESCRIPTION</label>
              <textarea
                value={nftDescription}
                onChange={(e) => setNftDescription(e.target.value)}
                className="w-full bg-[#1e1e2e] border border-[#45475a] p-2 text-[#cdd6f4] text-sm h-20 resize-none rounded focus:border-[#cba6f7] outline-none transition-colors"
                placeholder="Achievement unlocked for..."
              />
            </div>

            <button
              onClick={handleMint}
              disabled={!connected || !imageFile || !nftName || isMinting}
              className={`w-full border px-4 py-3 text-sm transition-all rounded ${
                !connected || !imageFile || !nftName || isMinting
                  ? "bg-[#1e1e2e] border-[#45475a] text-[#6c7086] cursor-not-allowed"
                  : "bg-[#313145] border-[#cba6f7] text-[#cba6f7] hover:bg-[#45475a]"
              }`}
            >
              {isMinting ? "[MINTING...]" : "[MINT NFT]"}
            </button>

            {mintStatus && (
              <div className="terminal-box p-3 text-xs">
                <div className="text-[#89b4fa] animate-pulse">{">"} {mintStatus}</div>
              </div>
            )}
          </div>
        </div>

        {/* Minted NFTs */}
        <div className="terminal-box p-5">
          <div className="text-[#cba6f7] mb-4 font-medium">MINTED NFTS [{mintedNFTs.length}]</div>

          {mintedNFTs.length === 0 ? (
            <div className="h-[400px] flex items-center justify-center text-[#6c7086] text-sm">
              <pre>
{`  ┌─────────────┐
  │  NO NFTS    │
  │  MINTED     │
  │  YET        │
  └─────────────┘`}
              </pre>
            </div>
          ) : (
            <div className="space-y-3 max-h-[500px] overflow-y-auto">
              {mintedNFTs.map((nft) => (
                <div key={nft.id} className="border border-[#45475a] p-3 rounded">
                  <div className="flex gap-3">
                    <div className="w-16 h-16 border border-[#45475a] flex-shrink-0 rounded overflow-hidden">
                      <img src={nft.imageUrl} alt={nft.name} className="w-full h-full object-cover" />
                    </div>
                    <div className="flex-1 min-w-0">
                      <div className="text-[#cba6f7] text-sm truncate">{nft.name}</div>
                      <div className="text-[#6c7086] text-xs mt-1">{nft.timestamp}</div>
                      <div className="text-[#6c7086] text-xs mt-1 truncate">
                        Mint: {nft.mintAddress}
                      </div>
                    </div>
                  </div>
                </div>
              ))}
            </div>
          )}

          <div className="mt-4 pt-4 border-t border-[#45475a] text-xs text-[#6c7086]">
            <div>{">"} Network: Solana Devnet</div>
            <div>{">"} Standard: Metaplex NFT</div>
          </div>
        </div>
      </div>
    </div>
  );
}
