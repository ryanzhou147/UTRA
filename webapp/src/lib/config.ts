// Application configuration
// Environment variables and feature flags

export const config = {
  // MongoDB
  mongodb: {
    uri: process.env.MONGODB_URI || "",
    database: process.env.MONGODB_DB || "utra",
  },

  // Solana
  solana: {
    rpcEndpoint: process.env.SOLANA_RPC || "https://api.devnet.solana.com",
    network: (process.env.SOLANA_NETWORK || "devnet") as "devnet" | "mainnet-beta",
    integrityProgramId: process.env.INTEGRITY_PROGRAM_ID || "",
    badgesProgramId: process.env.BADGES_PROGRAM_ID || "",
  },

  // Snowflake (future integration)
  snowflake: {
    account: process.env.SNOWFLAKE_ACCOUNT || "",
    username: process.env.SNOWFLAKE_USERNAME || "",
    password: process.env.SNOWFLAKE_PASSWORD || "",
    database: process.env.SNOWFLAKE_DATABASE || "UTRA_ANALYTICS",
    schema: process.env.SNOWFLAKE_SCHEMA || "PUBLIC",
    warehouse: process.env.SNOWFLAKE_WAREHOUSE || "COMPUTE_WH",
    enabled: process.env.SNOWFLAKE_ENABLED === "true",
  },

  // Feature flags
  features: {
    useMockData: process.env.USE_MOCK_DATA !== "false", // Default to mock
    enableNFTMinting: process.env.ENABLE_NFT_MINTING === "true",
    enableSolanaVerification: process.env.ENABLE_SOLANA_VERIFICATION === "true",
    enableSnowflakeSync: process.env.ENABLE_SNOWFLAKE_SYNC === "true",
  },

  // Arduino Bridge
  bridge: {
    serialPort: process.env.SERIAL_PORT || "/dev/tty.usbmodem14101",
    baudRate: parseInt(process.env.BAUD_RATE || "9600"),
  },
};

export default config;
