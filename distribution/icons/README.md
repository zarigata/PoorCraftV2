# Application Icons

This directory contains application icons for different platforms.

## Icon Files

- **poorcraft.ico** - Windows icon (multi-resolution: 16x16, 32x32, 48x48, 256x256)
- **poorcraft.icns** - macOS icon (multi-resolution: 16x16 to 512x512 @2x)
- **poorcraft-256.png** - Linux icon (256x256 PNG)

## Creating Icons

### Windows (.ico)

Use a tool like ImageMagick or GIMP to create a multi-resolution .ico file:

```bash
# Using ImageMagick
convert icon-16.png icon-32.png icon-48.png icon-256.png poorcraft.ico
```

### macOS (.icns)

Use the `iconutil` command on macOS:

```bash
# Create iconset directory
mkdir PoorCraft.iconset

# Add required sizes
cp icon-16.png PoorCraft.iconset/icon_16x16.png
cp icon-32.png PoorCraft.iconset/icon_16x16@2x.png
cp icon-32.png PoorCraft.iconset/icon_32x32.png
cp icon-64.png PoorCraft.iconset/icon_32x32@2x.png
cp icon-128.png PoorCraft.iconset/icon_128x128.png
cp icon-256.png PoorCraft.iconset/icon_128x128@2x.png
cp icon-256.png PoorCraft.iconset/icon_256x256.png
cp icon-512.png PoorCraft.iconset/icon_256x256@2x.png
cp icon-512.png PoorCraft.iconset/icon_512x512.png
cp icon-1024.png PoorCraft.iconset/icon_512x512@2x.png

# Generate .icns
iconutil -c icns PoorCraft.iconset
```

### Linux (.png)

Simply use a 256x256 PNG file. Most Linux desktop environments support PNG icons.

## Placeholder Icons

The current placeholder icons are simple colored squares with "PC" text.
Replace these with proper game icons before public release.

## Design Guidelines

- Use a simple, recognizable design
- Ensure the icon looks good at small sizes (16x16)
- Use high contrast for visibility
- Consider the game's theme and branding
- Test on different backgrounds (light/dark)
