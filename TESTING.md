# PoorCraft Testing Guide

## Phase 9 Test Plan

### Test Objectives
- Verify compilation of all modules
- Validate runtime execution
- Test core systems (rendering, world generation, networking, UI)
- Ensure cross-platform compatibility

---

## Compilation Tests

### Build All Modules
```bash
./gradlew clean build
```

**Expected Result**: All modules compile without errors
- `buildSrc` - Build plugin compiles
- `common` - Common code compiles
- `core` - Engine core compiles
- `modapi` - Mod API compiles
- `client` - Client compiles
- `server` - Server compiles

**Check for**:
- No compilation errors
- No critical warnings
- All dependencies resolved

---

## Client Tests

### Launch Client
```bash
./gradlew :client:run
```

**Test Checklist**:
- [ ] Window creates successfully
- [ ] OpenGL context initializes
- [ ] Main menu displays
- [ ] Settings menu accessible
- [ ] Can navigate UI with mouse/keyboard

### Singleplayer World Generation
1. Click "Singleplayer" in main menu
2. World should generate

**Test Checklist**:
- [ ] World generates without errors
- [ ] Player spawns at Y=80
- [ ] Chunks load around player
- [ ] Terrain renders correctly
- [ ] Sky/fog renders

### Player Movement
**Test Checklist**:
- [ ] WASD keys move player
- [ ] Mouse look rotates camera
- [ ] Space jumps
- [ ] Shift crouches/descends
- [ ] Movement is smooth (60 FPS target)
- [ ] Collision detection works

### Block Interaction
**Test Checklist**:
- [ ] Left-click breaks blocks
- [ ] Block breaking animation displays (crack textures)
- [ ] Broken blocks drop items
- [ ] Right-click places blocks
- [ ] Placed blocks render correctly
- [ ] Block updates propagate

### Inventory System
**Test Checklist**:
- [ ] Press E to open inventory
- [ ] Inventory UI displays
- [ ] Can move items between slots
- [ ] Hotbar displays at bottom
- [ ] Number keys (1-9) select hotbar slots
- [ ] Mouse wheel scrolls hotbar selection
- [ ] Selected slot highlights

### HUD Display
**Test Checklist**:
- [ ] Crosshair displays at screen center
- [ ] Health bar displays (hearts)
- [ ] Hunger bar displays (drumsticks)
- [ ] Hotbar displays with items
- [ ] Selected hotbar slot highlights

### Pause Menu
**Test Checklist**:
- [ ] ESC opens pause menu
- [ ] Game pauses (world stops updating)
- [ ] Can resume game
- [ ] Can access settings
- [ ] Can return to main menu
- [ ] Can quit game

### Chat System
**Test Checklist**:
- [ ] Press T to open chat
- [ ] Can type messages
- [ ] Enter sends message
- [ ] ESC cancels chat
- [ ] Chat history displays

### Hot-Reload
**Test Checklist**:
- [ ] Edit shader file while running
- [ ] Shader reloads automatically
- [ ] Edit texture file while running
- [ ] Texture reloads automatically
- [ ] No crashes during reload

---

## Server Tests

### Launch Server
```bash
./gradlew :server:run
```

**Test Checklist**:
- [ ] Server starts without errors
- [ ] Binds to port 25565
- [ ] World generates
- [ ] Console accepts commands
- [ ] Logs to `logs/` directory

### Server Commands
**Test in server console**:
```
stop          - Graceful shutdown
help          - List commands
list          - List connected players
```

**Test Checklist**:
- [ ] Commands execute successfully
- [ ] Server responds to commands
- [ ] Stop command shuts down cleanly

---

## Multiplayer Integration Tests

### Setup
1. Start server: `./gradlew :server:run`
2. Start client: `./gradlew :client:run --args='--multiplayer'`
3. Connect to `localhost:25565`

### Connection Tests
**Test Checklist**:
- [ ] Client connects to server
- [ ] Login succeeds
- [ ] Player spawns in world
- [ ] Server logs connection
- [ ] Client receives world data

### Movement Synchronization
**Test Checklist**:
- [ ] Player movement syncs to server
- [ ] Position updates broadcast
- [ ] Other players visible (if multiple clients)
- [ ] Movement is smooth
- [ ] No rubber-banding

### Block Interaction Sync
**Test Checklist**:
- [ ] Block breaks sync to server
- [ ] Block placements sync to server
- [ ] Changes visible to other clients
- [ ] Server validates actions
- [ ] Invalid actions rejected

### Chat Broadcasting
**Test Checklist**:
- [ ] Chat messages send to server
- [ ] Server broadcasts to all clients
- [ ] Messages display in chat overlay
- [ ] Player names display correctly

### Disconnect/Reconnect
**Test Checklist**:
- [ ] Client can disconnect cleanly
- [ ] Server logs disconnection
- [ ] Can reconnect after disconnect
- [ ] World state persists

---

## Performance Tests

### Frame Rate
**Target**: 60 FPS minimum

**Test Conditions**:
- Render distance: 8 chunks
- No VSync
- Default graphics settings

**Measure**:
- Press F3 for debug overlay
- Check FPS counter
- Monitor frame time

**Test Checklist**:
- [ ] Maintains 60+ FPS in open areas
- [ ] Maintains 45+ FPS in dense areas
- [ ] No stuttering during chunk loading
- [ ] Smooth camera movement

### Chunk Loading
**Test Checklist**:
- [ ] Chunks load within 2 seconds
- [ ] No visible pop-in
- [ ] Smooth loading as player moves
- [ ] Chunks unload properly
- [ ] Memory usage stable

### Memory Usage
**Monitor with**: Task Manager / Activity Monitor / `top`

**Test Checklist**:
- [ ] Initial memory < 1GB
- [ ] Memory stable after 10 minutes
- [ ] No memory leaks
- [ ] GC pauses < 50ms

### Network Performance
**Test with**: Server on localhost

**Test Checklist**:
- [ ] Ping < 10ms (localhost)
- [ ] No packet loss
- [ ] Updates arrive within 50ms
- [ ] Bandwidth < 1MB/s

---

## Known Issues

### Graphics
- **Reverse-Z depth**: Only works on OpenGL 4.5+ with `GL_ARB_clip_control`
- **Texture hot-reload**: May fail on Windows if file is locked

### Networking
- **Multiplayer**: Still in early development, expect bugs
- **NAT traversal**: Not implemented, LAN/localhost only

### Platform-Specific
- **macOS**: Requires `-XstartOnFirstThread` JVM flag (handled by Gradle)
- **Linux**: May need `libglfw3` and `libopenal1` packages
- **Windows**: Requires Visual C++ Redistributable

---

## Test Results Template

### Test Session
- **Date**: YYYY-MM-DD
- **Tester**: Name
- **Platform**: OS and version
- **Java Version**: X.X.X
- **Build**: Commit hash or version

### Results
| Test Category | Status | Notes |
|--------------|--------|-------|
| Compilation | ✓/✗ | |
| Client Launch | ✓/✗ | |
| World Generation | ✓/✗ | |
| Player Movement | ✓/✗ | |
| Block Interaction | ✓/✗ | |
| Inventory | ✓/✗ | |
| HUD | ✓/✗ | |
| Server Launch | ✓/✗ | |
| Multiplayer | ✓/✗ | |
| Performance | ✓/✗ | FPS: XX |

### Issues Found
1. **Issue description**
   - Severity: Critical/High/Medium/Low
   - Steps to reproduce
   - Expected vs actual behavior
   - Workaround (if any)

---

## Automated Testing

### Unit Tests
```bash
./gradlew test
```

**Coverage**:
- Common utilities
- Configuration loading
- World generation algorithms
- Networking protocols
- Entity systems

### Integration Tests
```bash
./gradlew integrationTest
```

**Coverage**:
- Client-server communication
- World loading/saving
- Mod loading
- Resource management

---

## Debugging Tips

### Enable Debug Logging
Edit `config/client.yml` or `config/server.yml`:
```yaml
logging:
  level: DEBUG
```

### Debug Overlay
Press **F3** in-game to show:
- FPS and frame time
- Position and rotation
- Chunk coordinates
- Memory usage
- OpenGL info

### Common Issues

**Black screen on launch**:
- Check GPU drivers are up to date
- Verify OpenGL 3.3+ support
- Check logs for shader compilation errors

**Textures missing**:
- Run `python scripts/generate_placeholder_textures.py`
- Verify `client/src/main/resources/textures/` exists
- Check logs for texture loading errors

**Font rendering errors**:
- Run `bash scripts/download_font.sh`
- Verify `fonts/default.ttf` exists
- Check font file is valid TrueType

**Configuration errors**:
- Verify `config/` directory is writable
- Check YAML syntax in config files
- Delete config to regenerate defaults

**Networking fails**:
- Check firewall allows port 25565
- Verify server is running
- Check server logs for errors
- Try localhost/127.0.0.1 instead of hostname

---

## Reporting Bugs

When reporting bugs, include:
1. **Description**: Clear description of the issue
2. **Steps to reproduce**: Exact steps to trigger the bug
3. **Expected behavior**: What should happen
4. **Actual behavior**: What actually happens
5. **Environment**: OS, Java version, GPU
6. **Logs**: Relevant log excerpts from `logs/`
7. **Screenshots**: If visual issue

Submit to: GitHub Issues or project bug tracker
