#!/usr/bin/env python3
"""
Generate placeholder textures for PoorCraft client.

Usage:
    python scripts/generate_placeholder_textures.py

This script generates all missing UI textures, skins, and test textures
using PIL/Pillow. All textures are created with appropriate dimensions
and basic visual patterns for testing purposes.
"""

import os
import sys
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("ERROR: PIL/Pillow not found. Install with: pip install Pillow")
    sys.exit(1)


def create_crosshair(size=16):
    """Create a simple white crosshair texture."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    center = size // 2
    thickness = 2
    length = 6
    
    # Vertical line
    draw.rectangle([center - thickness//2, center - length, 
                   center + thickness//2, center + length], fill=(255, 255, 255, 255))
    # Horizontal line
    draw.rectangle([center - length, center - thickness//2,
                   center + length, center + thickness//2], fill=(255, 255, 255, 255))
    
    return img


def create_heart(size=9, state='full'):
    """Create heart icon for health display."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    if state == 'full':
        # Red filled heart
        draw.polygon([(4, 2), (2, 0), (0, 2), (0, 4), (4, 8), (8, 4), (8, 2), (6, 0)], 
                    fill=(255, 0, 0, 255))
    elif state == 'half':
        # Half red, half outline
        draw.polygon([(4, 2), (2, 0), (0, 2), (0, 4), (4, 8)], fill=(255, 0, 0, 255))
        draw.polygon([(4, 2), (6, 0), (8, 2), (8, 4), (4, 8)], outline=(64, 0, 0, 255))
    else:  # empty
        # Dark outline only
        draw.polygon([(4, 2), (2, 0), (0, 2), (0, 4), (4, 8), (8, 4), (8, 2), (6, 0)], 
                    outline=(64, 0, 0, 255))
    
    return img


def create_hunger(size=9, state='full'):
    """Create hunger icon (drumstick)."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    if state == 'full':
        # Brown drumstick
        draw.ellipse([2, 1, 6, 5], fill=(139, 69, 19, 255))
        draw.rectangle([3, 4, 5, 7], fill=(139, 69, 19, 255))
    elif state == 'half':
        # Half filled
        draw.ellipse([2, 1, 4, 5], fill=(139, 69, 19, 255))
        draw.ellipse([4, 1, 6, 5], outline=(68, 34, 0, 255))
        draw.rectangle([3, 4, 4, 7], fill=(139, 69, 19, 255))
        draw.rectangle([4, 4, 5, 7], outline=(68, 34, 0, 255))
    else:  # empty
        # Dark outline only
        draw.ellipse([2, 1, 6, 5], outline=(68, 34, 0, 255))
        draw.rectangle([3, 4, 5, 7], outline=(68, 34, 0, 255))
    
    return img


def create_hotbar(width=182, height=22):
    """Create hotbar background with 9 slots."""
    img = Image.new('RGBA', (width, height), (64, 64, 64, 128))
    draw = ImageDraw.Draw(img)
    
    slot_size = 20
    spacing = 2
    start_x = 1
    
    for i in range(9):
        x = start_x + i * (slot_size + spacing)
        draw.rectangle([x, 1, x + slot_size, 1 + slot_size], outline=(128, 128, 128, 255))
    
    return img


def create_hotbar_selection(size=24):
    """Create selected hotbar slot highlight."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # White border with 2-pixel thickness
    draw.rectangle([0, 0, size-1, size-1], outline=(255, 255, 255, 255), width=2)
    
    return img


def create_slot(size=18):
    """Create inventory slot background."""
    img = Image.new('RGBA', (size, size), (48, 48, 48, 255))
    draw = ImageDraw.Draw(img)
    
    # Lighter border
    draw.rectangle([0, 0, size-1, size-1], outline=(96, 96, 96, 255))
    
    return img


def create_inventory_background(width=176, height=166):
    """Create inventory screen background."""
    img = Image.new('RGBA', (width, height), (32, 32, 32, 192))
    draw = ImageDraw.Draw(img)
    
    # Draw slot grid outlines
    slot_size = 18
    spacing = 2
    
    # Player inventory (9x4 grid at bottom)
    start_y = height - 4 * (slot_size + spacing) - 10
    for row in range(4):
        for col in range(9):
            x = 10 + col * (slot_size + spacing)
            y = start_y + row * (slot_size + spacing)
            draw.rectangle([x, y, x + slot_size, y + slot_size], outline=(96, 96, 96, 255))
    
    return img


def create_crack(size=16, stage=0):
    """Create block breaking crack texture."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Progressive cracking pattern
    intensity = int((stage + 1) * 25.5)  # 0-255 based on stage 0-9
    num_lines = stage + 2
    
    for i in range(num_lines):
        # Random-looking but deterministic crack lines
        x1 = (i * 7) % size
        y1 = (i * 11) % size
        x2 = ((i + 1) * 13) % size
        y2 = ((i + 1) * 17) % size
        
        draw.line([(x1, y1), (x2, y2)], fill=(0, 0, 0, 180), width=1)
    
    return img


def create_player_skin(size=32):
    """Create default player skin (Steve-style)."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    skin_color = (240, 192, 144)
    shirt_color = (64, 64, 255)
    pants_color = (32, 32, 160)
    
    # Head (8x8 at 0,0)
    draw.rectangle([0, 0, 7, 7], fill=skin_color)
    # Eyes
    draw.rectangle([2, 2, 3, 3], fill=(0, 0, 0))
    draw.rectangle([5, 2, 6, 3], fill=(0, 0, 0))
    
    # Body (8x12 at 16,16)
    draw.rectangle([16, 16, 23, 27], fill=shirt_color)
    
    # Right arm (4x12 at 40,16)
    draw.rectangle([40, 16, 43, 27], fill=skin_color)
    
    # Left arm (4x12 at 32,16)
    draw.rectangle([32, 16, 35, 27], fill=skin_color)
    
    # Right leg (4x12 at 0,16)
    draw.rectangle([0, 16, 3, 27], fill=pants_color)
    
    # Left leg (4x12 at 16,16) - overlaps body, adjust
    draw.rectangle([4, 16, 7, 27], fill=pants_color)
    
    return img


def create_npc_skin(size=32):
    """Create NPC villager skin."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    skin_color = (240, 192, 144)
    robe_color = (139, 105, 20)
    
    # Head (8x8 at 0,0)
    draw.rectangle([0, 0, 7, 7], fill=skin_color)
    # Large nose
    draw.rectangle([3, 4, 4, 6], fill=(200, 150, 100))
    # Unibrow
    draw.line([(1, 2), (6, 2)], fill=(80, 40, 0), width=1)
    
    # Body (8x12 at 16,16) - brown robe
    draw.rectangle([16, 16, 23, 27], fill=robe_color)
    
    # Arms and legs in robe color
    draw.rectangle([40, 16, 43, 27], fill=robe_color)
    draw.rectangle([32, 16, 35, 27], fill=robe_color)
    draw.rectangle([0, 16, 3, 27], fill=robe_color)
    draw.rectangle([4, 16, 7, 27], fill=robe_color)
    
    return img


def create_test_texture(size=256):
    """Create UV test grid texture."""
    img = Image.new('RGBA', (size, size), (255, 255, 255, 255))
    draw = ImageDraw.Draw(img)
    
    half = size // 2
    
    # Colored quadrants
    draw.rectangle([0, 0, half-1, half-1], fill=(255, 0, 0, 255))  # Red top-left
    draw.rectangle([half, 0, size-1, half-1], fill=(0, 255, 0, 255))  # Green top-right
    draw.rectangle([0, half, half-1, size-1], fill=(0, 0, 255, 255))  # Blue bottom-left
    draw.rectangle([half, half, size-1, size-1], fill=(255, 255, 0, 255))  # Yellow bottom-right
    
    # Grid lines
    grid_spacing = size // 8
    for i in range(0, size, grid_spacing):
        draw.line([(i, 0), (i, size)], fill=(0, 0, 0, 128), width=1)
        draw.line([(0, i), (size, i)], fill=(0, 0, 0, 128), width=1)
    
    return img


def main():
    # Get project root
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    resources_dir = project_root / "client" / "src" / "main" / "resources"
    textures_dir = resources_dir / "textures"
    ui_dir = textures_dir / "ui"
    skins_dir = textures_dir / "skins"
    
    # Create directories
    ui_dir.mkdir(parents=True, exist_ok=True)
    skins_dir.mkdir(parents=True, exist_ok=True)
    
    print("Generating placeholder textures...")
    
    # UI textures
    textures = [
        (ui_dir / "crosshair.png", create_crosshair()),
        (ui_dir / "heart_full.png", create_heart(9, 'full')),
        (ui_dir / "heart_half.png", create_heart(9, 'half')),
        (ui_dir / "heart_empty.png", create_heart(9, 'empty')),
        (ui_dir / "hunger_full.png", create_hunger(9, 'full')),
        (ui_dir / "hunger_half.png", create_hunger(9, 'half')),
        (ui_dir / "hunger_empty.png", create_hunger(9, 'empty')),
        (ui_dir / "hotbar.png", create_hotbar()),
        (ui_dir / "hotbar_selection.png", create_hotbar_selection()),
        (ui_dir / "slot.png", create_slot()),
        (ui_dir / "inventory_background.png", create_inventory_background()),
    ]
    
    # Crack textures
    for i in range(10):
        textures.append((ui_dir / f"crack_{i}.png", create_crack(16, i)))
    
    # Skin textures
    textures.extend([
        (skins_dir / "player_default.png", create_player_skin(32)),
        (skins_dir / "npc_villager.png", create_npc_skin(32)),
    ])
    
    # Test texture
    textures.append((textures_dir / "test.png", create_test_texture(256)))
    
    # Save all textures
    for path, img in textures:
        img.save(path)
        print(f"  Created: {path.relative_to(project_root)}")
    
    print(f"\nSuccessfully generated {len(textures)} placeholder textures!")


if __name__ == "__main__":
    main()
