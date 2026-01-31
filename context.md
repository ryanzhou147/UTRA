# UTRA Competition Platform - Architecture Plan

## Overview
A robotics competition platform that:
1. Logs team runs with scoring breakdowns and evidence (video/photo)
2. Shows live leaderboards and analytics
3. Records cryptographic proofs on Solana for tamper-proof scoring
4. Awards achievements as mintable Solana NFT badges

## Architecture

```
┌─────────────┐    Serial    ┌─────────────────┐
│   Arduino   │ ──────────── │  Bridge Server  │
│  (Sensors)  │    USB       │   (Node.js)     │
└─────────────┘              └────────┬────────┘
                                      │
                                      ▼
                             ┌─────────────────┐
                             │     MongoDB     │◄────────────────────────┐
                             │  (Heavy Data)   │                         │
                             └────────┬────────┘                         │
                                      │                                  │
                                      │ On "Verify on Chain":            │
                                      │ 1. Compute hash                  │
                                      │ 2. Write to Solana               │
                                      ▼                                  │
                             ┌─────────────────┐                         │
                             │     Solana      │      Verify:            │
                             │  (Integrity +   │──── Pull doc, rehash, ──┘
                             │   Badges/NFTs)  │      compare on-chain
                             └────────┬────────┘
                                      │
                                      ▼
                             ┌─────────────────┐
                             │    Next.js      │
                             │   Dashboard     │
                             └─────────────────┘
```

## Competition Sections

### 1. Target Shooting
Score breakdown:
- Dropoff points
- Rings hit
- Shot zone (Blue Zone bonus)
- Ball final position

### 2. Obstacle Course
Score breakdown:
- Completion time
- Obstacle touches (penalties)
- Resets used
- Reuploads

## Data Storage Strategy

### MongoDB (Heavy/Mutable Data)
**Database:** `utra`

**Collections:**

```js
// users
{
  _id: ObjectId,
  walletAddress: String,      // Solana pubkey (optional)
  name: String,
  email: String,
  avatar: String,
  role: "team" | "judge" | "admin",
  createdAt: Date
}

// teams
{
  _id: ObjectId,
  teamId: String,             // Human-readable ID (e.g., "TEAM-042")
  name: String,
  members: [ObjectId],        // Ref to users
  robotName: String,
  calibrationNotes: String,
  photos: [String],           // Robot photos
  videoLinks: [String],
  createdAt: Date
}

// runs
{
  _id: ObjectId,
  runId: String,              // UUID
  teamId: String,
  timestamp: Date,

  // Section
  section: "target_shooting" | "obstacle_course",
  isOfficial: Boolean,        // Official attempt vs practice

  // Target Shooting Scoring
  targetScoring: {
    dropoffPoints: Number,
    ringsHit: Number,
    shotZone: "blue" | "red" | "white" | "miss",
    ballPosition: String,     // Description or coordinates
    totalScore: Number
  },

  // Obstacle Course Scoring
  obstacleScoring: {
    completionTimeMs: Number,
    obstacleTouches: Number,  // Penalties
    resets: Number,
    reuploads: Number,
    timeBonus: Number,
    penaltyPoints: Number,
    totalScore: Number
  },

  // Evidence
  evidence: {
    videoUrl: String,         // Phone video link
    photoUrls: [String],      // Photos of final position
    notes: String             // Judge notes
  },

  // Telemetry (from Arduino sensors)
  telemetry: [{
    t: Number,                // ms since start
    distanceCm: Number,
    // ... other sensor data
  }],

  // Computed Metrics
  computedMetrics: {
    avgSpeed: Number,
    consistency: Number,      // Variance in runs
  },

  // Integrity (populated on "Verify on Chain")
  status: "draft" | "submitted" | "verified",
  onChainTx: String,          // Solana transaction signature
  onChainSlot: Number,
  integrityHash: String,      // SHA-256 of canonical fields
  verifiedAt: Date
}

// achievements
{
  _id: ObjectId,
  odtype: String,               // e.g., "curved_ramp_cleared"
  name: String,               // "Curved Ramp Cleared"
  description: String,
  icon: String,               // Emoji or image URL
  criteria: {                 // Auto-award logic
    section: String,
    field: String,            // e.g., "obstacleScoring.completionTimeMs"
    operator: "lt" | "gt" | "eq" | "gte" | "lte",
    value: Number
  }
}

// team_achievements
{
  _id: ObjectId,
  teamId: String,
  odtype: String,
  awardedAt: Date,
  runId: String,              // Run that triggered it
  mintTx: String,             // Solana NFT mint tx (optional)
  mintAddress: String         // NFT address (optional)
}

// sensor_logs (real-time stream)
{
  _id: ObjectId,
  timestamp: Date,
  sensorId: String,
  distanceCm: Number,
  distanceInch: Number,
  runId: String               // Null if not during a run
}
```

### Solana (Integrity Proofs + Achievement NFTs)

**1. Run Proof (PDA)**
```rust
// ~100 bytes per run
RunProof {
    run_id: [u8; 16],         // UUID bytes
    team_id: [u8; 32],        // Team identifier
    timestamp: i64,           // Unix timestamp
    section: u8,              // 0=target, 1=obstacle
    total_score: u32,         // Score * 100 (fixed point)
    data_hash: [u8; 32]       // SHA-256 of MongoDB doc
}
```

**2. Achievement Badge (NFT)**
```rust
// Metaplex NFT metadata
{
    name: "Under 60 Seconds",
    symbol: "UTRA",
    uri: "https://utra.app/achievements/under-60-seconds.json",
    attributes: [
        { trait_type: "Team", value: "TEAM-042" },
        { trait_type: "Run", value: "run-uuid" },
        { trait_type: "Time", value: "58.3s" }
    ]
}
```

## Achievement System

### Auto-Awarded Achievements (MongoDB Logic)

| Achievement | Criteria |
|-------------|----------|
| Curved Ramp Cleared | Obstacle course completed |
| Blue Zone Shot | `targetScoring.shotZone == "blue"` |
| Under 60 Seconds | `obstacleScoring.completionTimeMs < 60000` |
| Zero Resets | `obstacleScoring.resets == 0` |
| Perfect Run | No penalties + no resets |
| Sharpshooter | 3+ rings hit in one run |
| Consistency King | < 5% variance across 3+ runs |
| Most Improved | Score increased 20%+ from first to last run |

### Badge Minting Flow
```
Run Submitted → MongoDB checks criteria → Achievement awarded
                                              │
                                              ▼
                            User clicks "Mint as NFT"
                                              │
                                              ▼
                            Solana NFT minted to team wallet
                                              │
                                              ▼
                            mintTx saved to team_achievements
```

## Analytics & Leaderboards

### Live Scoreboard
- Ranked by total score per section
- Filter: Official runs only
- Shows verified badge for on-chain runs

### Team Analytics
- Best obstacle time
- Most consistent shot accuracy
- Fewest resets
- Improvement over attempts
- Timeline of progress across weekend

### Global Stats
- Fastest obstacle time (all teams)
- Highest target score
- Most achievements unlocked
- Most verified runs

## Project Structure

```
UTRA/
├── UTRA/                       # Arduino PlatformIO project
│   └── src/main.cpp
│
├── bridge/                     # Serial-to-MongoDB bridge
│   ├── package.json
│   ├── src/
│   │   ├── index.ts
│   │   ├── serial.ts
│   │   ├── parser.ts
│   │   └── db.ts
│   └── .env
│
├── contracts/                  # Solana programs (Anchor)
│   ├── Anchor.toml
│   └── programs/
│       ├── utra-integrity/
│       │   └── src/lib.rs     # RunProof storage
│       └── utra-badges/
│           └── src/lib.rs     # Achievement NFT minting
│
├── webapp/                     # Next.js dashboard
│   ├── package.json
│   ├── app/
│   │   ├── page.tsx           # Dashboard / Live scoreboard
│   │   ├── runs/
│   │   │   ├── new/
│   │   │   │   └── page.tsx   # Create new run
│   │   │   └── [runId]/
│   │   │       └── page.tsx   # Run detail + verify button
│   │   ├── teams/
│   │   │   ├── page.tsx       # All teams
│   │   │   └── [teamId]/
│   │   │       └── page.tsx   # Team profile + analytics
│   │   ├── leaderboard/
│   │   │   └── page.tsx       # Live rankings
│   │   ├── achievements/
│   │   │   └── page.tsx       # All achievements + who earned
│   │   └── api/
│   │       ├── runs/
│   │       │   ├── route.ts          # CRUD runs
│   │       │   └── [runId]/
│   │       │       └── verify/
│   │       │           └── route.ts  # Write to Solana
│   │       ├── teams/
│   │       │   └── route.ts
│   │       ├── leaderboard/
│   │       │   └── route.ts
│   │       ├── achievements/
│   │       │   ├── route.ts          # List achievements
│   │       │   └── mint/
│   │       │       └── route.ts      # Mint NFT badge
│   │       └── telemetry/
│   │           └── route.ts
│   ├── components/
│   │   ├── RunForm.tsx        # Score entry form
│   │   ├── ScoreCard.tsx
│   │   ├── Leaderboard.tsx
│   │   ├── DistanceChart.tsx
│   │   ├── LiveReading.tsx
│   │   ├── VerifyBadge.tsx
│   │   ├── AchievementCard.tsx
│   │   └── MintButton.tsx
│   └── lib/
│       ├── mongodb.ts         # Connection + collection types (IMPLEMENTED)
│       ├── solana.ts          # localStorage hash storage (IMPLEMENTED)
│       ├── config.ts          # App config + Snowflake placeholder (IMPLEMENTED)
│       └── achievements.ts    # Achievement checking logic
│
└── context.md
```

## Implementation Phases

### Phase 1: Core Platform
1. Set up MongoDB Atlas
2. Scaffold Next.js with Tailwind
3. Build team registration
4. Build run submission form (both sections)
5. Basic leaderboard

### Phase 2: Sensor Integration
1. Bridge server for Arduino serial
2. Live telemetry display
3. Attach telemetry to runs

### Phase 3: Solana Integrity
1. Write Anchor program for RunProof
2. Deploy to devnet
3. "Verify on Chain" button
4. Verification badge on scoreboard

### Phase 4: Achievements
1. Define achievement criteria in MongoDB
2. Auto-award on run submission
3. Achievement display on team profiles

### Phase 5: NFT Badges (Hackathon Wow)
1. Metaplex NFT collection setup
2. "Mint as NFT" button
3. Wallet connection (Phantom)
4. Badge gallery on team profile

## Verification Flow

```
Team submits run
        │
        ▼
Run saved to MongoDB (status: "submitted")
        │
        ▼
Judge reviews evidence
        │
        ▼
Click "Verify on Chain"
        │
        ▼
┌─────────────────────────────────┐
│ 1. Compute canonical hash       │
│ 2. Create Solana transaction    │
│ 3. Write RunProof to chain      │
│ 4. Save tx signature to MongoDB │
│ 5. Update status: "verified"    │
└─────────────────────────────────┘
        │
        ▼
Scoreboard shows ✅ Verified
```

## Environment Variables

**bridge/.env:**
```
MONGODB_URI=mongodb+srv://...
SERIAL_PORT=/dev/tty.usbmodem14101
BAUD_RATE=9600
```

**webapp/.env.local:** (see `.env.example`)
```
# MongoDB
MONGODB_URI=mongodb+srv://...
MONGODB_DB=utra

# Solana
SOLANA_RPC=https://api.devnet.solana.com
SOLANA_NETWORK=devnet
INTEGRITY_PROGRAM_ID=<deployed_program_id>
BADGES_PROGRAM_ID=<deployed_program_id>
ADMIN_WALLET_SECRET=<base58_keypair>

# Snowflake (future analytics)
SNOWFLAKE_ACCOUNT=
SNOWFLAKE_USERNAME=
SNOWFLAKE_DATABASE=UTRA_ANALYTICS
SNOWFLAKE_ENABLED=false

# Feature Flags
USE_MOCK_DATA=true
ENABLE_NFT_MINTING=false
ENABLE_SOLANA_VERIFICATION=false
```

## Local Hash Storage

Before MongoDB is connected, Solana verification hashes are stored in **localStorage**:

```typescript
// lib/solana.ts
interface StoredHash {
  runId: string;
  hash: string;        // SHA-256 of run data
  timestamp: number;
  verified: boolean;
  txSignature?: string;
}

// Functions available:
storeHash(runId, hash, txSignature?)
getHashForRun(runId)
markHashVerified(runId, txSignature)
exportHashesForSync()  // For migration to MongoDB
```

## Media Upload Strategy

MongoDB stores **URLs only**, not raw files. Actual media hosted externally:

| Media | Storage | Notes |
|-------|---------|-------|
| Videos | YouTube (unlisted) / Google Drive | Team pastes link, no upload needed |
| Photos | Uploadthing or Cloudinary | Free tier, CDN, Next.js integration |

**Why:** Videos are huge (100MB+), photos need optimization. Let dedicated services handle storage/CDN. MongoDB just references them.

```js
// Example run.evidence
evidence: {
  videoUrl: "https://youtube.com/watch?v=abc123",  // Pasted by team
  photoUrls: [
    "https://utfs.io/f/abc123.jpg",  // Uploaded via Uploadthing
    "https://utfs.io/f/def456.jpg"
  ],
  notes: "Ball landed 2cm from target"
}
```

## UI Theme: Chalk Terminal

Professional chalk terminal aesthetic - soft, muted colors inspired by Catppuccin.

### Design Direction
- Slate background (#1e1e2e, #282839)
- Muted purple accents (#cba6f7, #b4befe)
- Soft text colors (#cdd6f4, #a6adc8, #6c7086)
- Pastel status colors (green #a6e3a1, peach #fab387, red #f38ba8, blue #89b4fa)
- Monospace font (JetBrains Mono)
- Subtle scanline effect
- Rounded corners throughout

### Terminal UI Elements
```
┌──────────────────────────────────────────────┐
│  UTRA COMPETITION TERMINAL v1.0.0            │
│  ========================================    │
│                                              │
│  > LEADERBOARD                               │
│  ┌────┬────────────┬────────┬─────────────┐  │
│  │ #  │ TEAM       │ SCORE  │ STATUS      │  │
│  ├────┼────────────┼────────┼─────────────┤  │
│  │ 01 │ ROBO-042   │ 2,450  │ ✓ VERIFIED  │  │
│  │ 02 │ MECH-017   │ 2,180  │ ✓ VERIFIED  │  │
│  │ 03 │ BOLT-099   │ 1,920  │ ○ PENDING   │  │
│  └────┴────────────┴────────┴─────────────┘  │
│                                              │
│  > [ACHIEVEMENT UNLOCKED] BLUE_ZONE_SHOT     │
│  > Team ROBO-042 earned badge at 14:32:07    │
│                                              │
│  root@utra:~$ _                              │
└──────────────────────────────────────────────┘
```

### Key Pages in Terminal Style
- **Dashboard**: Live feed of runs, achievements, sensor data
- **Leaderboard**: ASCII table with rankings
- **Run Detail**: `cat run-123.json` style output
- **Verify**: Shows hash computation like a build log
- **Achievements**: Badge grid with `[LOCKED]` / `[UNLOCKED]`

## Tech Stack

| Component | Technology |
|-----------|------------|
| Microcontroller | Arduino Uno |
| Sensors | HC-SR04 Ultrasonic, SG90 Servo |
| Bridge | Node.js + serialport + TypeScript |
| Database | MongoDB Atlas |
| Blockchain | Solana (Anchor) |
| NFTs | Metaplex |
| Frontend | Next.js 14 + Tailwind |
| Terminal UI | react-terminal-ui |
| Charts | Recharts (styled to match terminal) |
| Media Upload | Uploadthing (photos) |
| Wallet | @solana/wallet-adapter + Phantom |

## Security Considerations

- Hash computed server-side (not client-tamperable)
- On-chain writes require admin wallet signature
- Evidence URLs validated (no arbitrary uploads)
- MongoDB append-only during competition
- Verification is public and auditable
- NFT badges are transferable but tied to run proof

## Webapp Usage

### Running the App
```bash
cd webapp
npm install
npm run dev
# Opens at http://localhost:3000
```

### Pages

| Route | Description |
|-------|-------------|
| `/` | Sensor dashboard - ultrasonic + color detection charts |
| `/runs` | Run history with video replay - click [+ ADD RUN] to add |
| `/achievements` | Badge grid - shows which teams earned each achievement |
| `/collection` | NFT storefront - browse minted badges by rarity |
| `/mint` | Mint new NFTs - connect wallet and upload image |

### Adding a Run
1. Go to `/runs`
2. Click `[+ ADD RUN]`
3. Select section (target/obstacle)
4. Enter score
5. Paste YouTube embed URL for video replay
6. Click `[SUBMIT]`

### Video URL Format
Use YouTube embed format:
```
https://www.youtube.com/embed/VIDEO_ID
```
Not regular YouTube URLs. Get embed URL from YouTube Share > Embed.

### Terminal Logs
The `/` page simulates live sensor data. To connect real Arduino:
1. Set up the bridge server (see bridge/ folder)
2. Configure serial port in bridge/.env
3. Logs will stream to the webapp via API

---

## Next Implementation Tasks

### UI Improvements
- [x] **Center the terminal** - Terminal centered on screen with `mx-auto`

### Sensor Dashboard Features (COMPLETED)
The main logs page (`/`) now has enhanced sensor visualization:

#### 1. Ultrasonic Sensor Logging
- [x] Real-time distance readings from HC-SR04 sensor
- [x] Track **sensor stability** - measure variance/jitter in readings
- [x] **Line chart**: Distance over time (Recharts)
- [x] **Stability indicator**: Rolling variance calculation with progress bar

#### 2. Color Detection Tracking
- [x] Track colors detected by color sensor
- [x] Colors: **Red, Green, Blue, Black**
- [x] **Bar graph**: Count of each color detected (horizontal bar chart)
- [x] **Live indicator**: Current detected color with color swatch

#### 3. Dashboard Layout
```
┌─────────────────────────────────────────────────────────────┐
│  UTRA SENSOR DASHBOARD                                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  [ULTRASONIC]                    [COLOR DETECTION]           │
│  ┌────────────────────┐          ┌────────────────────┐      │
│  │  Distance: 42.3cm  │          │  Current: ██ RED   │      │
│  │  Stability: 98.2%  │          │                    │      │
│  └────────────────────┘          └────────────────────┘      │
│                                                              │
│  [DISTANCE OVER TIME]            [COLOR BAR GRAPH]           │
│  ┌────────────────────┐          ┌────────────────────┐      │
│  │    📈 Line Chart   │          │  R: ████████ 42    │      │
│  │                    │          │  G: ██████ 31      │      │
│  │                    │          │  B: ████ 18        │      │
│  │                    │          │  K: ██ 9           │      │
│  └────────────────────┘          └────────────────────┘      │
│                                                              │
│  [TERMINAL LOG]                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  12:34:01 [SENSOR] Distance: 42.3cm                  │   │
│  │  12:34:02 [COLOR] Detected: RED                      │   │
│  │  12:34:03 [SENSOR] Distance: 41.8cm                  │   │
│  │  root@utra:~$ _                                      │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Data Structures for New Features

```typescript
// Ultrasonic reading with stability tracking
interface UltrasonicReading {
  timestamp: number;
  distanceCm: number;
}

interface UltrasonicStats {
  readings: UltrasonicReading[];
  avgDistance: number;
  variance: number;
  stability: number;  // 100 - (variance as percentage)
}

// Color detection
type DetectedColor = 'red' | 'green' | 'blue' | 'black';

interface ColorReading {
  timestamp: number;
  color: DetectedColor;
}

interface ColorStats {
  red: number;
  green: number;
  blue: number;
  black: number;
  total: number;
  currentColor: DetectedColor | null;
}
```

### Implementation Notes
- Use **Recharts** for graphs (already in tech stack, styled for terminal aesthetic)
- Stability = 100 - (coefficient of variation * 100), capped at 0-100%
- Rolling window of ~50 readings for stability calculation
- Color bar graph should update in real-time as new colors are detected

---

## Current Implementation Status

### Completed
- [x] Next.js webapp with chalk terminal theme
- [x] Sensor dashboard with ultrasonic + color detection charts
- [x] Runs page with team selection and video upload
- [x] Achievements page showing which teams earned badges
- [x] NFT collection/storefront page with rarity filtering
- [x] NFT minting page with Solana wallet integration
- [x] MongoDB scaffolding (`lib/mongodb.ts`) with all collection types
- [x] localStorage hash storage (`lib/solana.ts`) for offline verification
- [x] Config system (`lib/config.ts`) with Snowflake placeholder
- [x] Mock data for 5 teams, 8 runs, 8 NFTs

### Pending
- [ ] Connect MongoDB (copy `.env.example` to `.env.local`)
- [ ] API routes for CRUD operations
- [ ] Bridge server for Arduino serial
- [ ] Solana Anchor programs
- [ ] Snowflake analytics integration
