import { MongoClient, Db } from "mongodb";

// MongoDB connection scaffolding
// Set MONGODB_URI in .env.local to connect

const MONGODB_URI = process.env.MONGODB_URI || "";
const MONGODB_DB = process.env.MONGODB_DB || "utra";

let cachedClient: MongoClient | null = null;
let cachedDb: Db | null = null;

export async function connectToDatabase(): Promise<{ client: MongoClient; db: Db }> {
  if (cachedClient && cachedDb) {
    return { client: cachedClient, db: cachedDb };
  }

  if (!MONGODB_URI) {
    throw new Error("Please define MONGODB_URI environment variable");
  }

  const client = await MongoClient.connect(MONGODB_URI);
  const db = client.db(MONGODB_DB);

  cachedClient = client;
  cachedDb = db;

  return { client, db };
}

// Collection types matching context.md schema
export interface User {
  _id?: string;
  walletAddress?: string;
  name: string;
  email: string;
  avatar?: string;
  role: "team" | "judge" | "admin";
  createdAt: Date;
}

export interface Team {
  _id?: string;
  teamId: string;
  name: string;
  members: string[];
  robotName?: string;
  calibrationNotes?: string;
  photos: string[];
  videoLinks: string[];
  createdAt: Date;
}

export interface Run {
  _id?: string;
  runId: string;
  teamId: string;
  timestamp: Date;
  section: "target_shooting" | "obstacle_course";
  isOfficial: boolean;

  targetScoring?: {
    dropoffPoints: number;
    ringsHit: number;
    shotZone: "blue" | "red" | "white" | "miss";
    ballPosition: string;
    totalScore: number;
  };

  obstacleScoring?: {
    completionTimeMs: number;
    obstacleTouches: number;
    resets: number;
    reuploads: number;
    timeBonus: number;
    penaltyPoints: number;
    totalScore: number;
  };

  evidence: {
    videoUrl?: string;
    photoUrls: string[];
    notes?: string;
  };

  telemetry: Array<{
    t: number;
    distanceCm: number;
  }>;

  computedMetrics?: {
    avgSpeed: number;
    consistency: number;
  };

  // Integrity / Solana verification
  status: "draft" | "submitted" | "verified";
  onChainTx?: string;
  onChainSlot?: number;
  integrityHash?: string;
  verifiedAt?: Date;
}

export interface Achievement {
  _id?: string;
  type: string;
  name: string;
  description: string;
  icon: string;
  criteria: {
    section: string;
    field: string;
    operator: "lt" | "gt" | "eq" | "gte" | "lte";
    value: number;
  };
}

export interface TeamAchievement {
  _id?: string;
  teamId: string;
  type: string;
  awardedAt: Date;
  runId: string;
  mintTx?: string;
  mintAddress?: string;
}

export interface SensorLog {
  _id?: string;
  timestamp: Date;
  sensorId: string;
  distanceCm: number;
  distanceInch: number;
  runId?: string;
}

// NFT Collection item
export interface NFTItem {
  _id?: string;
  mintAddress: string;
  name: string;
  description: string;
  imageUrl: string;
  attributes: Array<{
    trait_type: string;
    value: string;
  }>;
  teamId: string;
  runId?: string;
  achievementType?: string;
  createdAt: Date;
}

// Helper to get collections
export async function getCollections(db: Db) {
  return {
    users: db.collection<User>("users"),
    teams: db.collection<Team>("teams"),
    runs: db.collection<Run>("runs"),
    achievements: db.collection<Achievement>("achievements"),
    teamAchievements: db.collection<TeamAchievement>("team_achievements"),
    sensorLogs: db.collection<SensorLog>("sensor_logs"),
    nfts: db.collection<NFTItem>("nfts"),
  };
}
