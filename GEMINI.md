# 🎯 GEMINI.md - Project Context, Architecture & Coding Standards

This file contains the foundational guidelines, architectural decisions, and coding standards for **gameFormation**. It is loaded automatically in every subsequent session to preserve full project context.

---

## 🎮 1. Project Overview & Environment

* **Goal**: Re-learn C/C++ by developing a visually engaging and robust **2D Zelda-like game**.
* **Target Audience**: Developed for a senior C# developer (10+ years of XP) transitioning to modern C++ (C++17).
* **Core Library**: **Raylib 5.0** (automatically managed and compiled via CMake `FetchContent`).
* **JSON Library**: **nlohmann/json 3.11.3** (automatically fetched via CMake with corporate proxy bypass `TLS_VERIFY FALSE`).
* **Multi-Language Switcher**: VS Code is optimized using **VS Code Profiles** to prevent active extension bloat when switching between C/C++, C#, and Java:
  - **"C++ Game Dev" Profile**: Enabled: `C/C++`, `CMake Tools`. Disabled: C#, Java.
  - **"C# .NET" Profile**: Enabled: `C#`, `C# Dev Kit`. Disabled: C++, Java.
  - **"Java" Profile**: Enabled: `Extension Pack for Java`. Disabled: C++, C#.

---

## 🏛️ 2. Workspace Architecture

The workspace is configured as a multi-target build system using CMake. It contains two main executable targets:
1. **`gameFormation`** (Main Game): Renders a title introduction screen, game world, controls documentation, and RPG weapon smithing.
2. **`gameplay_sandbox`** (Sandbox): An isolated playground featuring a C++17 filesystem scan map selector and an advanced Boomerang physics prototype.

Both targets share identical core modules linked in `CMakeLists.txt`:

```text
gameFormation/
├── .vscode/               # VS Code configurations (launch, settings, extensions)
├── assets/                # ASSETS STORAGE DIRECTORY
│   └── maps/
│       ├── game/          # Overworld maps used in the main game
│       │   └── overworld.json
│       └── sandbox/       # Test map files scanned dynamically by the Sandbox
│           ├── sandbox_default.json
│           ├── test_arena.json
│           └── giant_adventure.json (1600x1200 scrolling map)
│
├── markdown/
│   └── CodingStandard.md  # Official C++ Coding Standards (Zelda-like with Raylib)
├── src/
│   ├── core/              # GAME SYSTEM CONTROLLERS
│   │   ├── player.h/.cpp       # Player: movement, animations, attack hitbox scaling
│   │   ├── inventory.h         # Item & Inventory systems, spent points, Wide Enums
│   │   ├── game_world.h/.cpp   # GameWorld: camera, entities, and ground pickups resolver
│   │   └── game.h/.cpp         # Main Game: screens state machine (Title, Game, Options)
│   │
│   ├── map/               # MAP AND RENDERING MODULES
│   │   ├── tile_map.h/.cpp     # Dynamic TileMap: linear vector layout, AABB collision glides
│   │   ├── map_data.h          # Map level structures, spawn metadata
│   │   └── map_loader.h/.cpp   # JSON Parser: Decodes Tiled JSON files
│   │
│   ├── main.cpp           # Main game entry point (Window initialization)
│   └── sandbox.cpp        # Sandbox entry point (Filesystem scanner, boomerang prototype)
└── CMakeLists.txt         # Root build script defining both targets & Fetches
```

---

## ⚔️ 3. Core Gameplay Systems

### A. Zelda-Style RPG Inventory & Upgrade Forge (Touche 'I' / Select)
* **Item Collection**: The player starts the game with NO weapons. They must walk over ground-spawning pickups (**Sword** or **Boomerang**) to collect them via AABB collision detection. Action triggers (slashing or throwing) remain blocked until the respective item is collected.
* **Upgrade Forge**: Pressing **`I`** (or gamepad `Select`) freezes the game world (pause state) and opens a full, mouse-interactive Blacksmith Forge panel.
* **Stat Progression**: Players spend forge points to upgrade item level, damage, range (sword), speed (boomerang), or cycle magical elements:
  - **None (Physical)**: Standard physical damage (drawn as red hitbox).
  - **Fire**: Fire magic element (drawn as orange/red hitbox).
  - **Ice**: Ice freeze magic element (drawn as cyan/skyblue hitbox).
  - **Lightning**: Lightning element (drawn as electric gold/yellow hitbox).

### B. Dynamic Map Loader (Tiled JSON Support)
* **MapLoader**: Fully parses any JSON map file exported directly from **Tiled Map Editor**. It extracts the tile arrays, reads solid wall tiles into a physical boolean collision mask, and reads custom object layers containing entities (e.g. `PlayerSpawn`, pickup items, coordinates).
* **Fallback Safety**: If no custom collision layer is present in the Tiled JSON, `MapLoader` automatically deduces standard physical collisions based on tile IDs (IDs 1 and 2 are solid), ensuring out-of-the-box retrocompatibility.
* **Sandbox Filesystem Scan**: When `gameplay_sandbox` is launched, it uses C++17 `<filesystem>` to scan `assets/maps/sandbox/` for all `.json` files, offering an interactive list on the sandbox title screen.
* **Robust Multi-Path Fallback**: To support launching the binary from various working directories (e.g. terminal root, `/build`, or `/build/Debug`), `GetSandboxMaps` tests multiple path roots dynamically (`assets/maps/sandbox`, `../assets/maps/sandbox`, `../../assets/maps/sandbox`) and dynamically locks onto the verified working root directory.

### C. Camera2D Follow & Smooth Scrolling Follow
* The game world rendering is wrapped inside a **`Camera2D`** controller.
* **Smooth Scrolling (Lerp)**: The camera follows the player's movements using a smooth linear interpolation formula (`0.1f` damping), allowing them to explore giant maps (like `giant_adventure.json`, twice the size of an 800x600 screen) while remaining comfortably centered.
* All screen-space HUD panels, notifications, and menus are rendered outside of `BeginMode2D` to remain perfectly static on the screen.

---

## 📜 4. Mandatory Coding Standards (Custom C++17)

All files under `src/` must adhere strictly to the standards defined in `markdown/CodingStandard.md`. The key mandates to preserve in every future change are:

### A. Indentation & Formatting
* **Indent using 4 SPACES only**, never tabs.
* **Allman Braces Style**: Opening braces `{` are always on a new line:
  ```cpp
  if (condition)
  {
      doSomething();
  }
  ```
* Line length limit: **120 characters**.

### B. Naming Conventions
* **Classes, Structs, Enums**: `PascalCase` (e.g. `Player`, `TileMap`, `PlayerState`).
* **Methods / Functions**: `PascalCase` (e.g. `Update(float deltaTime)`, `Draw() const`).
* **Variables / Parameters**: `camelCase` (e.g. `deltaTime`, `collisionRect`).
* **Private Members**: Prefix with `m_` followed by `camelCase` (e.g. `m_position`, `m_speed`).
* **Constants**: Prefix with `k` followed by `PascalCase` (e.g. `kTileSize`, `kMapWidth`).
* **Files**: `snake_case` (e.g. `player.h`, `player.cpp`, `tile_map.h`, `tile_map.cpp`).

### C. Architecture, Const-Correctness, and Memory
* **Headers**: Use `#pragma once` (no manual include guards). Use forward declarations where possible.
* **Const-Correctness**: Const overloads must be supplied where appropriate. For example, `Inventory::GetItem()` must have both a non-const version (returning `Item*` for upgrades) and a const-qualified version (returning `const Item*` for drawing/rendering on a `const Player&`).
* **Heap Allocation**: Always prefer stack allocation. If dynamic allocation is necessary, use `std::make_unique<>` only. Never use bare `new` or `delete`.
* **Exceptions**: Interdicted (`-fno-exceptions`). Use standard asserts (`assert` / `ASSERT_MSG`).
* **Memory Diagnostics**: MSVC AddressSanitizer (`/fsanitize=address`) is integrated inside `CMakeLists.txt` to automatically detect heap leaks, out-of-bounds array accesses, and double-free corruptions during debug runs.
* **Yoda Conditions**: Removed. Compare normally: `if (m_state == PlayerState::Attacking)`.
* **Post-increment/decrement (`++` / `--`)**: Allowed where appropriate (e.g., standard loop counters or index incrementing).
