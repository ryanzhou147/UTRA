// Solana hash storage using localStorage
// Stores verification hashes locally until MongoDB is connected

import { createHash } from "crypto";

const STORAGE_KEY = "utra_solana_hashes";

export interface StoredHash {
  runId: string;
  hash: string;
  timestamp: number;
  verified: boolean;
  txSignature?: string;
}

// Get all stored hashes
export function getStoredHashes(): StoredHash[] {
  if (typeof window === "undefined") return [];
  const data = localStorage.getItem(STORAGE_KEY);
  return data ? JSON.parse(data) : [];
}

// Store a new hash
export function storeHash(runId: string, hash: string, txSignature?: string): void {
  if (typeof window === "undefined") return;
  const hashes = getStoredHashes();
  const existing = hashes.findIndex((h) => h.runId === runId);

  const entry: StoredHash = {
    runId,
    hash,
    timestamp: Date.now(),
    verified: !!txSignature,
    txSignature,
  };

  if (existing >= 0) {
    hashes[existing] = entry;
  } else {
    hashes.push(entry);
  }

  localStorage.setItem(STORAGE_KEY, JSON.stringify(hashes));
}

// Get hash for a specific run
export function getHashForRun(runId: string): StoredHash | undefined {
  return getStoredHashes().find((h) => h.runId === runId);
}

// Mark a hash as verified with tx signature
export function markHashVerified(runId: string, txSignature: string): void {
  const hashes = getStoredHashes();
  const entry = hashes.find((h) => h.runId === runId);
  if (entry) {
    entry.verified = true;
    entry.txSignature = txSignature;
    localStorage.setItem(STORAGE_KEY, JSON.stringify(hashes));
  }
}

// Compute integrity hash for run data
export function computeRunHash(runData: {
  runId: string;
  teamId: string;
  timestamp: number;
  section: string;
  score: number;
}): string {
  const canonical = JSON.stringify({
    runId: runData.runId,
    teamId: runData.teamId,
    timestamp: runData.timestamp,
    section: runData.section,
    score: runData.score,
  });

  // Use SHA-256 for integrity hash
  return createHash("sha256").update(canonical).digest("hex");
}

// Clear all stored hashes (for testing)
export function clearStoredHashes(): void {
  if (typeof window === "undefined") return;
  localStorage.removeItem(STORAGE_KEY);
}

// Export hashes for sync to MongoDB
export function exportHashesForSync(): StoredHash[] {
  return getStoredHashes().filter((h) => h.verified);
}
