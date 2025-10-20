# Particle Textures

This directory contains particle sprite textures for the particle system.

## Required Textures

Create the following 32×32 PNG textures with alpha channel:

- **smoke.png**: Gray smoke cloud (color #808080 to #C0C0C0 gradient), soft wispy appearance with transparency
- **fire.png**: Orange-red-yellow gradient flame (color #FF4500 to #FFFF00), pointed at top, wider at bottom
- **explosion.png**: Bright orange-yellow burst (color #FF8C00 to #FFFF00), irregular explosive shape
- **water_splash.png**: Light blue water droplets (color #87CEEB), multiple small droplets or splash shape
- **magic.png**: Purple-pink sparkle (color #9370DB to #FF69B4), star or sparkle shape with soft glow

## Creating Textures

You can create these textures using:
- Image editors: GIMP, Photoshop, Aseprite
- Procedural generation: radial gradients with noise
- Free resources: OpenGameArt.org, Kenney.nl

All textures should have:
- Size: 32×32 pixels
- Format: PNG with alpha channel
- Soft edges for smooth blending
- High contrast for visibility

## Usage

The ParticleSystem loads these textures into a texture atlas for efficient rendering.
