#!/usr/bin/env python3

from PIL import Image
import os

# Define block textures with their colors (RGB)
block_textures = {
    'stone.png': (128, 128, 128),      # Gray
    'dirt.png': (139, 69, 19),         # Brown
    'grass_top.png': (34, 139, 34),    # Green
    'grass_side.png': (139, 69, 19),   # Dirt brown (for sides)
    'sand.png': (238, 203, 173),       # Light yellow/beige
    'sandstone.png': (210, 180, 140),  # Tan
    'snow.png': (255, 250, 250),       # White
    'ice.png': (173, 216, 230),        # Light blue
    'wood_top.png': (160, 82, 45),     # Brown
    'wood_side.png': (139, 69, 19),    # Darker brown
    'leaves.png': (34, 139, 34),       # Green
    'water.png': (70, 130, 180)        # Steel blue
}

# Output directory
output_dir = '/home/zarigata/Projects/PoorCraftV2/client/src/main/resources/textures/blocks'

# Create 16x16 PNG files for each texture
for filename, color in block_textures.items():
    filepath = os.path.join(output_dir, filename)

    # Create a 16x16 image with the specified color
    img = Image.new('RGB', (16, 16), color=color)

    # Save as PNG
    img.save(filepath, 'PNG')
    print(f'Created {filepath}')

print(f'Created {len(block_textures)} block texture files')
