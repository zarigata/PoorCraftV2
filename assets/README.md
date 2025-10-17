# Assets Directory

This directory contains all game assets used by the PoorCraft engine. Assets are loaded at runtime by the ResourceManager system and are organized into specific subdirectories for different types of content.

## Directory Structure

```
assets/
├── textures/          # Block textures, entity skins, UI elements
├── models/            # 3D models for entities and objects
├── sounds/            # Audio files (music, sound effects)
├── fonts/             # TrueType fonts for UI text
└── README.md          # This file
```

## Textures (`textures/`)

Contains all texture files used in the game:

### Block Textures
- **Format**: PNG, 16x16 or 32x32 pixels recommended
- **Naming**: `blocks/stone.png`, `blocks/dirt.png`, `blocks/grass.png`
- **Purpose**: Textures for voxel blocks in the world

### Entity Skins
- **Format**: PNG, 32x32 pixels (as per project specification)
- **Naming**: `skins/steve.png`, `skins/alex.png`, `entities/creeper.png`
- **Purpose**: Character and mob textures

### UI Elements
- **Format**: PNG, various sizes as needed
- **Naming**: `ui/inventory.png`, `ui/crosshair.png`, `ui/hotbar.png`
- **Purpose**: Interface graphics and HUD elements

### Organization
```
textures/
├── blocks/            # World block textures (16x16 or 32x32)
├── skins/             # Player and entity skins (32x32)
├── entities/          # Mob and creature textures
├── items/             # Item and tool textures
├── ui/                # User interface elements
└── particles/         # Particle effect textures
```

## Models (`models/`)

Contains 3D model files for entities and objects:

- **Formats**: OBJ, FBX, or custom formats supported by the engine
- **Naming**: `entities/player.obj`, `blocks/cube.obj`
- **Purpose**: 3D geometry data for rendering

## Sounds (`sounds/`)

Contains audio files for game sound effects and music:

- **Formats**: OGG (recommended), WAV, MP3
- **Compression**: OGG provides good compression with minimal quality loss
- **Naming**: `player/footstep.ogg`, `music/background.ogg`, `effects/explosion.ogg`

### Organization
```
sounds/
├── music/             # Background music tracks
├── player/            # Player-related sounds (footsteps, hurt, death)
├── blocks/            # Block interaction sounds (break, place)
├── entities/          # Mob and creature sounds
├── effects/           # Special effects (explosions, magic)
└── ui/                # Interface sound effects (clicks, notifications)
```

## Fonts (`fonts/`)

Contains TrueType font files for UI text rendering:

- **Formats**: TTF, OTF
- **Naming**: `main.ttf`, `ui.ttf`, `monospace.ttf`
- **Sizes**: Include multiple sizes if needed for different UI contexts

## Asset Guidelines

### File Formats
- **Images**: PNG for textures with transparency, JPG for photographs
- **Audio**: OGG for compressed audio, WAV for short sound effects
- **Fonts**: TTF for maximum compatibility
- **Models**: OBJ with MTL for materials

### Naming Conventions
- Use lowercase letters and underscores
- Be descriptive and consistent
- Include category prefixes (blocks/, entities/, ui/)
- Use file extensions consistently

### Quality Standards
- **Textures**: Power-of-two dimensions when possible (16x16, 32x32, 64x64, etc.)
- **Audio**: 44.1kHz sample rate, 16-bit depth recommended
- **Colors**: sRGB color space for consistency

### Performance Considerations
- Compress textures where possible while maintaining quality
- Use appropriate audio compression levels
- Consider texture atlases for better rendering performance
- Keep file sizes reasonable for web/mobile deployment

## Asset Pipeline

### Loading
Assets are loaded at runtime through the ResourceManager:
- Automatic format detection based on file extension
- Asynchronous loading for large assets
- Caching and memory management

### Hot Reloading
During development, assets can be reloaded without restarting the engine:
- Monitor file modification times
- Automatically reload changed assets
- Preserve existing asset references

## Adding New Assets

1. Place files in the appropriate subdirectory
2. Follow the naming conventions
3. Update asset registration if needed (usually automatic)
4. Test loading and rendering

## Asset Sources

### Free Assets
- **OpenGameArt.org** - Free game art and textures
- **Freesound.org** - Free sound effects and music
- **Google Fonts** - Free fonts for UI

### Custom Assets
- Create original assets using tools like:
  - GIMP or Photoshop for textures
  - Audacity for audio
  - Blender for 3D models
  - FontForge for fonts

## Licensing

All assets must be compatible with the project's MIT license or be original creations. When using third-party assets, ensure proper attribution and license compliance.

## Examples

```
textures/blocks/stone.png          # Stone block texture
textures/skins/steve.png           # Default player skin
sounds/music/ambient.ogg           # Background music
fonts/ui.ttf                       # UI font file
```

For more information about the asset loading system, see the ResourceManager documentation.
