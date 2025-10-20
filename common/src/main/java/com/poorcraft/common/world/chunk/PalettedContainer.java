package com.poorcraft.common.world.chunk;

import com.poorcraft.common.Constants;

import java.util.ArrayList;
import java.util.List;

/**
 * Palette-based block storage following Minecraft's post-1.16 format.
 * Uses variable bits-per-entry encoding with indirect palette or direct storage.
 */
public class PalettedContainer {
    
    private List<Integer> palette;
    private long[] data;
    private int bitsPerEntry;
    private final int size;
    
    /**
     * Creates a new paletted container.
     * 
     * @param size Number of entries (typically 4096 for 16x16x16)
     */
    public PalettedContainer(int size) {
        this.size = size;
        this.palette = new ArrayList<>();
        this.palette.add(0); // Air is always index 0
        this.bitsPerEntry = Constants.World.MIN_PALETTE_BITS;
        this.data = new long[calculateDataLength(size, bitsPerEntry)];
    }
    
    /**
     * Gets a value at the specified index.
     * 
     * @param index Index in YZX order
     * @return Block ID
     */
    public int get(int index) {
        if (index < 0 || index >= size) {
            return 0;
        }
        
        int paletteIndex = extractFromData(index);
        
        if (bitsPerEntry >= Constants.World.MAX_PALETTE_BITS) {
            // Direct mode - value is the block ID itself
            return paletteIndex;
        } else {
            // Indirect mode - look up in palette
            if (paletteIndex >= 0 && paletteIndex < palette.size()) {
                return palette.get(paletteIndex);
            }
            return 0;
        }
    }
    
    /**
     * Sets a value at the specified index.
     * 
     * @param index Index in YZX order
     * @param value Block ID
     */
    public void set(int index, int value) {
        if (index < 0 || index >= size) {
            return;
        }
        
        int paletteIndex;
        
        if (bitsPerEntry >= Constants.World.MAX_PALETTE_BITS) {
            // Direct mode
            paletteIndex = value;
        } else {
            // Indirect mode - find or add to palette
            paletteIndex = palette.indexOf(value);
            if (paletteIndex == -1) {
                paletteIndex = palette.size();
                palette.add(value);
                
                // Check if we need to resize
                int requiredBits = calculateRequiredBits(palette.size());
                if (requiredBits > bitsPerEntry) {
                    resize(requiredBits);
                }
            }
        }
        
        writeToData(index, paletteIndex);
    }
    
    /**
     * Checks if this container is empty (all air).
     */
    public boolean isEmpty() {
        if (palette.size() == 1 && palette.get(0) == 0) {
            return true;
        }
        
        // Check if all entries are 0
        for (long datum : data) {
            if (datum != 0) {
                return false;
            }
        }
        return true;
    }
    
    /**
     * Compacts the palette to optimize storage.
     */
    public void compact() {
        if (bitsPerEntry >= Constants.World.MAX_PALETTE_BITS) {
            return; // Already in direct mode
        }
        
        // Count actual usage
        boolean[] used = new boolean[palette.size()];
        for (int i = 0; i < size; i++) {
            int paletteIndex = extractFromData(i);
            if (paletteIndex < used.length) {
                used[paletteIndex] = true;
            }
        }
        
        // Build new palette
        List<Integer> newPalette = new ArrayList<>();
        int[] remapping = new int[palette.size()];
        for (int i = 0; i < palette.size(); i++) {
            if (used[i]) {
                remapping[i] = newPalette.size();
                newPalette.add(palette.get(i));
            }
        }
        
        // Remap data
        int newBits = calculateRequiredBits(newPalette.size());
        if (newBits != bitsPerEntry) {
            long[] newData = new long[calculateDataLength(size, newBits)];
            for (int i = 0; i < size; i++) {
                int oldPaletteIndex = extractFromData(i);
                int newPaletteIndex = remapping[oldPaletteIndex];
                writeToData(newData, newBits, i, newPaletteIndex);
            }
            this.data = newData;
            this.bitsPerEntry = newBits;
        } else {
            // Just remap in place
            for (int i = 0; i < size; i++) {
                int oldPaletteIndex = extractFromData(i);
                int newPaletteIndex = remapping[oldPaletteIndex];
                writeToData(i, newPaletteIndex);
            }
        }
        
        this.palette = newPalette;
    }
    
    /**
     * Gets memory usage in bytes.
     */
    public long getMemoryUsage() {
        return (long) data.length * 8 + (long) palette.size() * 4;
    }
    
    private int extractFromData(int index) {
        int entriesPerLong = 64 / bitsPerEntry;
        int longIndex = index / entriesPerLong;
        int bitOffset = (index % entriesPerLong) * bitsPerEntry;
        
        if (longIndex >= data.length) {
            return 0;
        }
        
        long mask = (1L << bitsPerEntry) - 1;
        return (int) ((data[longIndex] >>> bitOffset) & mask);
    }
    
    private void writeToData(int index, int value) {
        writeToData(data, bitsPerEntry, index, value);
    }
    
    private void writeToData(long[] dataArray, int bits, int index, int value) {
        int entriesPerLong = 64 / bits;
        int longIndex = index / entriesPerLong;
        int bitOffset = (index % entriesPerLong) * bits;
        
        if (longIndex >= dataArray.length) {
            return;
        }
        
        long mask = (1L << bits) - 1;
        dataArray[longIndex] = (dataArray[longIndex] & ~(mask << bitOffset)) | ((long) value << bitOffset);
    }
    
    private void resize(int newBits) {
        if (newBits >= Constants.World.MAX_PALETTE_BITS) {
            // Switch to direct mode
            newBits = Constants.World.MAX_PALETTE_BITS;
            long[] newData = new long[calculateDataLength(size, newBits)];
            
            for (int i = 0; i < size; i++) {
                int paletteIndex = extractFromData(i);
                int blockId = palette.get(paletteIndex);
                writeToData(newData, newBits, i, blockId);
            }
            
            this.data = newData;
            this.bitsPerEntry = newBits;
            this.palette = null; // No longer needed in direct mode
        } else {
            // Resize within indirect mode
            long[] newData = new long[calculateDataLength(size, newBits)];
            
            for (int i = 0; i < size; i++) {
                int paletteIndex = extractFromData(i);
                writeToData(newData, newBits, i, paletteIndex);
            }
            
            this.data = newData;
            this.bitsPerEntry = newBits;
        }
    }
    
    private int calculateRequiredBits(int paletteSize) {
        if (paletteSize >= Constants.World.DIRECT_PALETTE_THRESHOLD) {
            return Constants.World.MAX_PALETTE_BITS;
        }
        
        int bits = Constants.World.MIN_PALETTE_BITS;
        while ((1 << bits) < paletteSize && bits < Constants.World.MAX_PALETTE_BITS) {
            bits++;
        }
        return bits;
    }
    
    private int calculateDataLength(int size, int bitsPerEntry) {
        int entriesPerLong = 64 / bitsPerEntry;
        return (size + entriesPerLong - 1) / entriesPerLong;
    }
}
