package com.poorcraft.common.world.chunk;

import com.poorcraft.common.Constants;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class PalettedContainerTest {

    @Test
    void directModeEmptyChecksHandleNullPalette() {
        int size = 512;
        PalettedContainer container = new PalettedContainer(size);

        // Populate with enough unique values to trigger direct mode
        for (int i = 0; i <= Constants.World.DIRECT_PALETTE_THRESHOLD; i++) {
            container.set(i, i);
        }

        assertFalse(container.isEmpty(), "Container with non-zero entries should not be empty");

        for (int i = 0; i < size; i++) {
            container.set(i, 0);
        }

        assertTrue(container.isEmpty(), "Container filled with zeros should be empty");

        int bitsPerEntry = Constants.World.MAX_PALETTE_BITS;
        int entriesPerLong = 64 / bitsPerEntry;
        int dataLength = (size + entriesPerLong - 1) / entriesPerLong;
        long expectedBytes = (long) dataLength * Long.BYTES;

        assertEquals(expectedBytes, container.getMemoryUsage(),
                "Direct mode memory usage should only include packed data array");
    }
}
