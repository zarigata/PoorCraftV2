# Test Texture

Place a `test.png` file in this directory for testing the rendering pipeline.

The texture should be:
- 256x256 pixels (or any power-of-2 size)
- PNG format with RGBA channels
- A recognizable pattern (e.g., UV test grid, checkerboard, or logo)

Example patterns:
- **UV Test Grid**: Colored quadrants (red top-left, green top-right, blue bottom-left, yellow bottom-right)
- **Checkerboard**: Alternating colored squares
- **Simple Gradient**: To verify texture sampling

If no texture is provided, the TextureManager will use a magenta/black checkerboard as a fallback.
