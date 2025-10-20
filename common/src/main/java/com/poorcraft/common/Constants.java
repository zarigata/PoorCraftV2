package com.poorcraft.common;

/**
 * Project-wide constants.
 */
public final class Constants {
    
    private Constants() {
        // Prevent instantiation
    }
    
    /**
     * Game metadata constants.
     */
    public static final class Game {
        public static final String NAME = "PoorCraft";
        public static final String VERSION = "0.1.0-SNAPSHOT";
        public static final int PROTOCOL_VERSION = 1;
        
        private Game() {}
    }
    
    /**
     * World-related constants.
     */
    public static final class World {
        public static final int CHUNK_SIZE_X = 16;
        public static final int CHUNK_SIZE_Y = 256;
        public static final int CHUNK_SIZE_Z = 16;
        public static final int SEA_LEVEL = 64;
        public static final int MIN_RENDER_DISTANCE = 2;
        public static final int MAX_RENDER_DISTANCE = 32;
        public static final int DEFAULT_RENDER_DISTANCE = 8;
        
        private World() {}
    }
    
    /**
     * Player-related constants.
     */
    public static final class Player {
        public static final float HEIGHT = 1.8f;
        public static final float WIDTH = 0.6f;
        public static final float EYE_HEIGHT = 1.62f;
        public static final float REACH_DISTANCE = 5.0f;
        public static final float WALK_SPEED = 4.317f; // blocks per second
        public static final float SPRINT_SPEED = 5.612f;
        public static final float SNEAK_SPEED = 1.295f;
        
        private Player() {}
    }
    
    /**
     * Skin-related constants.
     */
    public static final class Skin {
        public static final int TEXTURE_WIDTH = 32;
        public static final int TEXTURE_HEIGHT = 32;
        
        private Skin() {}
    }
    
    /**
     * File path constants.
     */
    public static final class Paths {
        public static final String SAVES_DIR = "saves";
        public static final String MODS_DIR = "mods";
        public static final String CONFIG_DIR = "config";
        public static final String LOGS_DIR = "logs";
        public static final String RESOURCES_DIR = "resources";
        public static final String SCREENSHOTS_DIR = "screenshots";
        
        private Paths() {}
    }
    
    /**
     * Network-related constants.
     */
    public static final class Network {
        public static final int DEFAULT_SERVER_PORT = 25565;
        public static final int CONNECTION_TIMEOUT_MS = 30000;
        public static final int MAX_PACKET_SIZE = 2097152; // 2MB
        public static final int COMPRESSION_THRESHOLD = 256;
        
        private Network() {}
    }
}
