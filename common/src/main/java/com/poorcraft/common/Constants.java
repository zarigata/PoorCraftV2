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
        public static final int CHUNK_SECTION_SIZE = 16;
        public static final int SECTIONS_PER_CHUNK = CHUNK_SIZE_Y / CHUNK_SECTION_SIZE;
        public static final int BLOCKS_PER_CHUNK = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;
        public static final int MIN_PALETTE_BITS = 4;
        public static final int MAX_PALETTE_BITS = 15;
        public static final int DIRECT_PALETTE_THRESHOLD = 256;
        
        private World() {}
    }
    
    /**
     * Block ID constants.
     */
    public static final class Blocks {
        public static final int AIR_ID = 0;
        public static final int STONE_ID = 1;
        public static final int DIRT_ID = 2;
        public static final int GRASS_ID = 3;
        public static final int SAND_ID = 4;
        public static final int SANDSTONE_ID = 5;
        public static final int SNOW_ID = 6;
        public static final int ICE_ID = 7;
        public static final int WOOD_ID = 8;
        public static final int LEAVES_ID = 9;
        public static final int WATER_ID = 10;
        
        private Blocks() {}
    }
    
    /**
     * Biome ID constants.
     */
    public static final class Biomes {
        public static final int PLAINS_ID = 0;
        public static final int DESERT_ID = 1;
        public static final int MOUNTAINS_ID = 2;
        public static final int SNOW_ID = 3;
        public static final int JUNGLE_ID = 4;
        public static final int BIOME_COUNT = 5;
        
        private Biomes() {}
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
     * Entity system constants.
     */
    public static final class Entity {
        public static final float DEFAULT_SPAWN_DISTANCE = 128.0f;
        public static final float MAX_RENDER_DISTANCE = 96.0f;
        public static final float DESPAWN_DISTANCE = 144.0f;
        public static final float UPDATE_RADIUS = 64.0f;
        public static final int PLAYER_ENTITY_ID = 0;
        public static final int NPC_ENTITY_ID = 1;

        private Entity() {}
    }

    /**
     * Animation-related constants.
     */
    public static final class Animation {
        public static final int DEFAULT_TICK_RATE = 20;
        public static final float INTERPOLATION_THRESHOLD = 0.001f;

        private Animation() {}
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
    
    /**
     * Rendering-related constants.
     */
    public static final class Rendering {
        public static final int TARGET_UPS = 60;
        public static final double FIXED_TIMESTEP = 1.0 / 60.0;
        public static final double MAX_FRAME_TIME = 0.25;
        public static final float DEFAULT_FOV = 70.0f;
        public static final float DEFAULT_NEAR_PLANE = 0.1f;
        public static final float DEFAULT_FAR_PLANE = 1000.0f;
        public static final int OPENGL_MAJOR = 3;
        public static final int OPENGL_MINOR = 3;
        
        private Rendering() {}
    }
    
    /**
     * Input-related constants.
     */
    public static final class Input {
        public static final int MOUSE_BUTTON_LEFT = 0;
        public static final int MOUSE_BUTTON_RIGHT = 1;
        public static final int MOUSE_BUTTON_MIDDLE = 2;
        public static final int MAX_GAMEPADS = 4;
        public static final float DEFAULT_DEADZONE = 0.25f;
        
        private Input() {}
    }
}
