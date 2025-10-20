package com.poorcraft.api.event;

/**
 * Base class for all events in the PoorCraft event system.
 * 
 * <p>Events are used to notify mods about game occurrences and allow them
 * to modify behavior. Mods can listen to events using event handlers.
 * 
 * <p>Example event implementation:
 * <pre>{@code
 * public class BlockBreakEvent extends Event {
 *     private final Block block;
 *     private final Player player;
 *     
 *     public BlockBreakEvent(Block block, Player player) {
 *         this.block = block;
 *         this.player = player;
 *     }
 *     
 *     public Block getBlock() { return block; }
 *     public Player getPlayer() { return player; }
 * }
 * }</pre>
 * 
 * <p>The event bus and specific event types will be implemented in Phase 6.
 */
public abstract class Event {
    
    private boolean cancelled = false;
    
    /**
     * Checks if this event can be cancelled.
     * 
     * @return true if the event is cancellable
     */
    public boolean isCancellable() {
        return this instanceof Cancellable;
    }
    
    /**
     * Checks if this event has been cancelled.
     * 
     * @return true if the event is cancelled
     */
    public boolean isCancelled() {
        return cancelled;
    }
    
    /**
     * Cancels this event if it is cancellable.
     * 
     * @throws UnsupportedOperationException if the event is not cancellable
     */
    public void setCancelled(boolean cancelled) {
        if (!isCancellable()) {
            throw new UnsupportedOperationException("Event " + getClass().getSimpleName() + " is not cancellable");
        }
        this.cancelled = cancelled;
    }
    
    /**
     * Marker interface for cancellable events.
     */
    public interface Cancellable {
    }
}
