# PoorCraft Mod API

The PoorCraft Mod API provides interfaces and utilities for creating mods that extend the game's functionality.

## Overview

The modapi module is the public API for mod developers. It provides:
- Mod lifecycle management through the `@Mod` annotation
- Event system for hooking into game events
- Interfaces for custom blocks, items, entities, and more (to be added in Phase 6)

## Getting Started

### Setting Up a Mod Project

1. Create a new Gradle or Maven project
2. Add the PoorCraft modapi as a dependency:

**Gradle:**
```gradle
dependencies {
    compileOnly 'com.poorcraft:poorcraft-modapi:0.1.0-SNAPSHOT'
}
```

**Maven:**
```xml
<dependency>
    <groupId>com.poorcraft</groupId>
    <artifactId>poorcraft-modapi</artifactId>
    <version>0.1.0-SNAPSHOT</version>
    <scope>provided</scope>
</dependency>
```

### Creating Your First Mod

1. Create a main mod class and annotate it with `@Mod`:

```java
package com.example.mymod;

import com.poorcraft.api.Mod;

@Mod(
    id = "mymod",
    name = "My First Mod",
    version = "1.0.0",
    description = "My first PoorCraft mod!",
    authors = {"YourName"}
)
public class MyMod {
    // Mod implementation will go here
}
```

2. Package your mod as a JAR file
3. Place the JAR in the `mods/` directory
4. Launch PoorCraft

## API Structure

### Packages

- **com.poorcraft.api** - Core mod API, including the `@Mod` annotation
- **com.poorcraft.api.event** - Event system for listening to game events
- **com.poorcraft.api.block** - Block API (Phase 6)
- **com.poorcraft.api.item** - Item API (Phase 6)
- **com.poorcraft.api.entity** - Entity API (Phase 6)
- **com.poorcraft.api.world** - World generation API (Phase 6)

## Mod Lifecycle

Mods go through the following lifecycle stages:

1. **Discovery** - The mod loader scans for `@Mod` annotated classes
2. **Construction** - Mod instances are created
3. **Initialization** - Mods register their content and event handlers
4. **Post-Initialization** - Cross-mod integration occurs
5. **Runtime** - Mods respond to events during gameplay
6. **Shutdown** - Mods clean up resources

## Event System

The event system allows mods to respond to game events:

```java
// Example event handler (Phase 6 will add the handler registration mechanism)
public void onBlockBreak(BlockBreakEvent event) {
    if (event.getBlock().getType() == Blocks.DIAMOND_ORE) {
        // Custom logic when diamond ore is broken
        event.getPlayer().sendMessage("You found diamonds!");
    }
}
```

### Event Priority

Event handlers can specify priority to control execution order:
- **LOWEST** - Runs first
- **LOW**
- **NORMAL** (default)
- **HIGH**
- **HIGHEST** - Runs last

### Cancellable Events

Some events can be cancelled to prevent the default behavior:

```java
public void onBlockBreak(BlockBreakEvent event) {
    if (someCondition) {
        event.setCancelled(true); // Prevent the block from breaking
    }
}
```

## Best Practices

1. **Use Unique Mod IDs** - Choose a unique, descriptive ID for your mod
2. **Semantic Versioning** - Follow semantic versioning (MAJOR.MINOR.PATCH)
3. **Declare Dependencies** - List all mod dependencies in the `@Mod` annotation
4. **Handle Errors Gracefully** - Don't crash the game; log errors instead
5. **Optimize Performance** - Be mindful of performance in event handlers
6. **Document Your API** - If your mod provides an API, document it well
7. **Test Compatibility** - Test your mod with other popular mods

## API Stability

The modapi follows semantic versioning:
- **Major version** changes may break compatibility
- **Minor version** changes add features without breaking compatibility
- **Patch version** changes are bug fixes only

We strive to maintain backward compatibility within major versions.

## Examples

Example mods can be found in the `mods/` directory of the main repository:
- **example_lua_mod** - Example Lua-based mod
- **example_native_mod** - Example Java-based mod
- **event_logger_mod** - Demonstrates event system usage

## Support

- **Documentation**: See `docs/MODDING.md` for detailed modding guide
- **API Docs**: Javadoc is available in the `-javadoc.jar` artifact
- **Community**: Join our Discord server for help and discussion

## License

The PoorCraft Mod API is licensed under the same license as PoorCraft itself.
See the LICENSE file in the root directory for details.
