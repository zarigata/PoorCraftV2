package com.poorcraft.common.network;

import com.poorcraft.common.network.packet.HandshakePacket;
import com.poorcraft.common.network.packet.LoginStartPacket;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for packet encoding and decoding.
 */
public class PacketTest {

    @Test
    public void testHandshakePacket() {
        HandshakePacket original = new HandshakePacket(340, "localhost", 25565, 2);

        ByteBuf buf = Unpooled.buffer();
        original.write(buf);

        HandshakePacket decoded = new HandshakePacket();
        decoded.read(buf);

        assertEquals(original.getProtocolVersion(), decoded.getProtocolVersion());
        assertEquals(original.getServerAddress(), decoded.getServerAddress());
        assertEquals(original.getServerPort(), decoded.getServerPort());
        assertEquals(original.getNextState(), decoded.getNextState());
    }

    @Test
    public void testLoginStartPacket() {
        String username = "TestPlayer";
        LoginStartPacket original = new LoginStartPacket(username);

        ByteBuf buf = Unpooled.buffer();
        original.write(buf);

        LoginStartPacket decoded = new LoginStartPacket();
        decoded.read(buf);

        assertEquals(original.getUsername(), decoded.getUsername());
    }

    @Test
    public void testNetworkUtilVarInt() {
        ByteBuf buf = Unpooled.buffer();

        // Test various varint values
        NetworkUtil.writeVarInt(buf, 0);
        assertEquals(0, NetworkUtil.readVarInt(buf));

        buf.clear();
        NetworkUtil.writeVarInt(buf, 127);
        assertEquals(127, NetworkUtil.readVarInt(buf));

        buf.clear();
        NetworkUtil.writeVarInt(buf, 25565);
        assertEquals(25565, NetworkUtil.readVarInt(buf));
    }

    @Test
    public void testNetworkUtilString() {
        ByteBuf buf = Unpooled.buffer();

        String testString = "Hello, World!";
        NetworkUtil.writeString(buf, testString);

        String decoded = NetworkUtil.readString(buf);
        assertEquals(testString, decoded);
    }
}
