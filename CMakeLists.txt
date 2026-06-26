cmake_minimum_required(VERSION 3.15)
project(gameFormation CXX)

# Évite les erreurs sur les versions récentes de CMake (3.30+)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Standard C++ 17 (très confortable pour un développeur C# moderne)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionnel: Évite les warnings de sécurité Microsoft dépréciés sur Windows
# et active l'AddressSanitizer (détection de fuites et corruptions mémoire)
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/fsanitize=address)
    add_link_options(/INCREMENTAL:NO) # Désactive l'édition de liens incrémentale pour supprimer le warning LNK4300

    # Résout le répertoire binaire MSVC et copie automatiquement les DLLs ASan nécessaires.
    # Cela évite l'erreur de lancement 0xc0000135 (STATUS_DLL_NOT_FOUND).
    get_filename_component(MSVC_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ASAN_DLL_NAME "clang_rt.asan_dynamic-x86_64.dll")
        set(ASAN_DBG_DLL_NAME "clang_rt.asan_dbg_dynamic-x86_64.dll")
    else()
        set(ASAN_DLL_NAME "clang_rt.asan_dynamic-i386.dll")
        set(ASAN_DBG_DLL_NAME "clang_rt.asan_dbg_dynamic-i386.dll")
    endif()

    foreach(CONFIG_DIR "Debug" "Release" "RelWithDebInfo")
        if(EXISTS "${MSVC_BIN_DIR}/${ASAN_DLL_NAME}")
            file(COPY "${MSVC_BIN_DIR}/${ASAN_DLL_NAME}" DESTINATION "${CMAKE_BINARY_DIR}/${CONFIG_DIR}")
        endif()
        if(EXISTS "${MSVC_BIN_DIR}/${ASAN_DBG_DLL_NAME}")
            file(COPY "${MSVC_BIN_DIR}/${ASAN_DBG_DLL_NAME}" DESTINATION "${CMAKE_BINARY_DIR}/${CONFIG_DIR}")
        endif()
    endforeach()
endif()

# FetchContent pour récupérer et compiler Raylib et nlohmann/json de manière autonome
include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
    TLS_VERIFY FALSE
)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    TLS_VERIFY FALSE
)

# Configuration de Raylib pour ne compiler que la bibliothèque statique
set(build_examples OFF CACHE BOOL "" FORCE)
set(build_games OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib json)

# 📦 Liste des fichiers sources partagés (le cœur du moteur de jeu)
set(CORE_SOURCES
    src/core/player.h
    src/core/player.cpp
    src/core/inventory.h
    src/core/ground_pickup.h
    src/core/hud.h
    src/core/hud.cpp
    src/core/destructible.h
    src/core/destructible.cpp
    src/core/npc.h
    src/core/npc.cpp
    src/core/enemy.h
    src/core/enemy.cpp
    src/core/enemy_behavior.h
    src/core/enemy_behavior.cpp
    src/core/entity_manager.h
    src/core/entity_manager.cpp
    src/core/dialogue_system.h
    src/core/dialogue_system.cpp
    src/core/event_system.h
    src/core/event_system.cpp
    src/core/combat_system.h
    src/core/combat_system.cpp
    src/core/physics_system.h
    src/core/physics_system.cpp
    src/core/spatial_grid.h
    src/core/spatial_grid.cpp
    src/core/camera_controller.h
    src/core/camera_controller.cpp
    src/core/scene.h
    src/core/scene_manager.h
    src/core/scene_manager.cpp
    src/core/title_scene.h
    src/core/title_scene.cpp
    src/core/gameplay_scene.h
    src/core/gameplay_scene.cpp
    src/core/options_scene.h
    src/core/options_scene.cpp
    src/core/inventory_scene.h
    src/core/inventory_scene.cpp
    src/core/game_world.h
    src/core/game_world.cpp
    src/core/game.h
    src/core/game.cpp
    src/map/tile_map.h
    src/map/tile_map.cpp
    src/map/map_data.h
    src/map/map_loader.h
    src/map/map_loader.cpp
)

# 🎯 CIBLE 1 (Jeu Principal) : Le Jeu Zelda 2D
add_executable(gameFormation src/main.cpp ${CORE_SOURCES})
target_link_libraries(gameFormation PRIVATE raylib nlohmann_json::nlohmann_json)
target_include_directories(gameFormation PRIVATE src ${raylib_SOURCE_DIR}/src)

# 🎯 CIBLE 2 (Sous-projet / Sandbox) : Le bac à sable pour expérimenter le gameplay
add_executable(gameplay_sandbox src/sandbox.cpp ${CORE_SOURCES})
target_link_libraries(gameplay_sandbox PRIVATE raylib nlohmann_json::nlohmann_json)
target_include_directories(gameplay_sandbox PRIVATE src ${raylib_SOURCE_DIR}/src)
