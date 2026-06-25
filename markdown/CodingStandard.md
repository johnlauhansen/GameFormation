# C++ Coding Standards — Zelda-like with Raylib
> C++17 strict · Raylib · Adapté aux développeurs venant de C#

Ce document est la référence à fournir à ton LLM pour générer du code cohérent.
Il traduit les habitudes C# en idiomes C++ modernes sûrs et performants.

---

## 1. Philosophie générale

| Principe | C# (ton réflexe) | C++ équivalent ici |
|---|---|---|
| Garbage collector | `new MyClass()` sans se soucier | `std::unique_ptr<>` ou allocation sur la pile |
| Interface / abstract | `interface IUpdatable` | classe de base avec méthode virtuelle pure |
| `null` check | `if (obj == null)` | `if (!ptr)` ou `std::optional<>` |
| `foreach` | `foreach (var e in list)` | `for (const auto& e : list)` |
| `string` | `System.String` | `std::string` (ou `std::string_view` en lecture seule) |
| Properties | `public float Health { get; set; }` | méthode `GetHealth()` / `SetHealth()` |
| Exception | `throw new Exception(...)` | **Pas d'exceptions.** Utiliser `assert` + codes retour. |

---

## 2. Naming & Formatting

### 2.1 Conventions de nommage

```cpp
// Classes, structs, enums → PascalCase
class PlayerEntity {};
struct TileData {};
enum class Direction { Up, Down, Left, Right };

// Méthodes et fonctions → PascalCase (cohérent avec Raylib et l'habitude C#)
void Update(float deltaTime);
bool IsGrounded() const;

// Variables locales et paramètres → camelCase
float deltaTime = 0.0f;
int tileIndex = 0;

// Membres privés → préfixe m_ + camelCase
class PlayerEntity {
private:
    float m_health;
    Vector2 m_position;  // type Raylib
};

// Constantes et constexpr → kPascalCase
constexpr int kTileSize = 16;
constexpr float kGravity = 9.8f;

// Fichiers → snake_case
// player_entity.h / player_entity.cpp
// tile_map.h / tile_map.cpp
```

### 2.2 Formatting

- Indentation : **4 espaces** (pas de tabulations)
- Accolades : **style Allman** (accolade ouvrante sur la ligne suivante) — proche du C# par défaut
- Longueur de ligne : **120 caractères** maximum
- Un seul espace autour des opérateurs : `x = a + b`, pas `x=a+b`

```cpp
// ✅ Correct
void PlayerEntity::Update(float deltaTime)
{
    if (m_health <= 0.0f)
    {
        Die();
        return;
    }

    m_position.x += m_velocity.x * deltaTime;
    m_position.y += m_velocity.y * deltaTime;
}

// ❌ Éviter
void PlayerEntity::Update(float deltaTime){
    if(m_health<=0.0f){Die();return;}
    m_position.x+=m_velocity.x*deltaTime;
}
```

### 2.3 Includes

```cpp
// Ordre dans chaque .cpp :
// 1. Header correspondant (en premier, pour détecter les dépendances manquantes)
#include "player_entity.h"

// 2. Headers du projet
#include "components/health_component.h"
#include "core/game_world.h"

// 3. Librairies tierces
#include <raylib.h>

// 4. STL
#include <vector>
#include <memory>
#include <string>
```

---

## 3. Gestion mémoire

### 3.1 Règle générale : préférer la pile

```cpp
// ✅ Sur la pile — quasi-gratuit, durée de vie automatique
TileData tile;
Vector2 spawnPos = { 100.0f, 200.0f };

// ✅ Conteneur sur la pile avec taille connue
std::array<int, 16> localBuffer;
```

### 3.2 Propriété unique → `std::unique_ptr`

Équivalent du `new` C# quand **un seul propriétaire** détient l'objet.

```cpp
// ✅ Création (pas de new brut)
auto player = std::make_unique<PlayerEntity>();

// ✅ Transfert de propriété
std::unique_ptr<PlayerEntity> TakeOwnership(std::unique_ptr<PlayerEntity> p)
{
    p->Respawn();
    return p;  // move implicite
}

// ✅ Passage en paramètre sans transfert : passer un pointeur brut non-owning
void RenderEntity(const PlayerEntity* entity)
{
    // entity n'est pas propriétaire — ne pas delete
}
player->Render();        // accès via ->
RenderEntity(player.get());  // .get() donne le pointeur brut
```

### 3.3 Propriété partagée → `std::shared_ptr` (avec modération)

```cpp
// Utiliser uniquement quand plusieurs systèmes partagent réellement la propriété
// Ex : une texture chargée et partagée entre plusieurs sprites
std::shared_ptr<Texture2D> sharedTexture = std::make_shared<Texture2D>(LoadTexture("hero.png"));

// ⚠️ shared_ptr a un coût (compteur atomique) — ne pas l'utiliser par défaut
```

### 3.4 Références non-owning → pointeur brut ou référence

```cpp
// Référence non-owning : raw pointer (pas de delete, pas de propriété)
class HealthComponent
{
public:
    explicit HealthComponent(PlayerEntity* owner) : m_owner(owner) {}

private:
    PlayerEntity* m_owner;  // non-owning, pas de delete ici
};

// Référence constante pour les paramètres lus (comme `in` en C#)
void DrawHealth(const HealthComponent& health);
```

### 3.5 Conteneurs — préférer `std::vector`

```cpp
// ✅ Vecteur de valeurs (pas de pointeurs si possible — meilleur cache)
std::vector<EnemyEntity> m_enemies;

// ✅ Vecteur de unique_ptr si les objets sont polymorphiques
std::vector<std::unique_ptr<Entity>> m_entities;

// ✅ Réserver à l'avance si la taille est connue
m_enemies.reserve(64);

// ❌ Éviter std::list (mauvais cache, rarement justifié)
```

### 3.6 Pas d'allocation dans le hot path

```cpp
// ❌ Mauvais — allocation chaque frame
void Update(float dt)
{
    std::vector<Enemy*> visibleEnemies;  // allocation heap chaque Update()
    // ...
}

// ✅ Bon — pré-alloué comme membre, réutilisé chaque frame
class GameWorld
{
    std::vector<Enemy*> m_visibleEnemies;  // réservé une fois en init

    void Update(float dt)
    {
        m_visibleEnemies.clear();  // clear() ne libère pas la mémoire
        // remplissage...
    }
};
```

---

## 4. Patterns — Architecture Zelda-like

### 4.1 Hiérarchie d'entités (approche simple, sans ECS complet)

Pour un jeu solo personnel, une hiérarchie de classes avec composants optionnels est
plus accessible qu'un ECS pur.

```
Entity                  (base : position, update, draw)
  ├── LivingEntity      (+ health, faction)
  │     ├── PlayerEntity
  │     └── EnemyEntity
  └── PropEntity        (coffre, porte, interrupteur)
```

```cpp
// entity.h
#pragma once
#include <raylib.h>

class Entity
{
public:
    Entity() = default;
    virtual ~Entity() = default;

    // Non-copiable (comme les MonoBehaviour Unity)
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // Déplaçable
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw() const = 0;

    [[nodiscard]] Vector2 GetPosition() const { return m_position; }
    void SetPosition(Vector2 pos) { m_position = pos; }

    [[nodiscard]] bool IsAlive() const { return m_alive; }

protected:
    Vector2 m_position = { 0.0f, 0.0f };
    bool m_alive = true;
};
```

### 4.2 Composants (composition over inheritance)

```cpp
// components/health_component.h
#pragma once

class HealthComponent
{
public:
    explicit HealthComponent(float maxHealth)
        : m_maxHealth(maxHealth), m_currentHealth(maxHealth) {}

    void TakeDamage(float amount);
    void Heal(float amount);

    [[nodiscard]] float GetHealth() const { return m_currentHealth; }
    [[nodiscard]] float GetMaxHealth() const { return m_maxHealth; }
    [[nodiscard]] bool IsDead() const { return m_currentHealth <= 0.0f; }

private:
    float m_maxHealth;
    float m_currentHealth;
};

// Usage dans LivingEntity
class LivingEntity : public Entity
{
public:
    explicit LivingEntity(float maxHealth)
        : m_health(maxHealth) {}

protected:
    HealthComponent m_health;  // composition, pas d'héritage
};
```

### 4.3 State Machine (états du joueur)

```cpp
// player_state.h
#pragma once

// Forward declaration
class PlayerEntity;

class PlayerState
{
public:
    virtual ~PlayerState() = default;
    virtual void Enter(PlayerEntity& player) {}
    virtual void Exit(PlayerEntity& player) {}
    virtual void Update(PlayerEntity& player, float deltaTime) = 0;
    virtual void Draw(const PlayerEntity& player) const = 0;
};

// États concrets
class IdleState    : public PlayerState { /* ... */ };
class WalkState    : public PlayerState { /* ... */ };
class AttackState  : public PlayerState { /* ... */ };
class DashState    : public PlayerState { /* ... */ };
```

```cpp
// Dans PlayerEntity
class PlayerEntity : public LivingEntity
{
public:
    void ChangeState(std::unique_ptr<PlayerState> newState);

private:
    std::unique_ptr<PlayerState> m_currentState;
};

void PlayerEntity::ChangeState(std::unique_ptr<PlayerState> newState)
{
    if (m_currentState)
        m_currentState->Exit(*this);

    m_currentState = std::move(newState);
    m_currentState->Enter(*this);
}
```

### 4.4 Tile Map

```cpp
// tile_map.h
#pragma once
#include <vector>
#include <array>
#include <raylib.h>

struct TileData
{
    int textureIndex = 0;
    bool isSolid = false;
    bool isWater = false;
};

class TileMap
{
public:
    TileMap(int width, int height);

    void Draw() const;
    [[nodiscard]] bool IsSolid(int tileX, int tileY) const;
    [[nodiscard]] TileData& GetTile(int tileX, int tileY);

    static constexpr int kTileSize = 16;

private:
    [[nodiscard]] int ToIndex(int x, int y) const { return y * m_width + x; }

    int m_width;
    int m_height;
    std::vector<TileData> m_tiles;  // layout linéaire, cache-friendly
};
```

---

## 5. Erreurs & Asserts

### 5.1 Pas d'exceptions — assert en debug

```cpp
// core/assert.h
#pragma once
#include <cassert>

// ASSERT(condition) — actif uniquement en debug (NDEBUG désactivé)
// Équivalent de Debug.Assert() en C#
#define ASSERT(cond) assert(cond)

// ASSERT_MSG(condition, message) — avec message lisible
#define ASSERT_MSG(cond, msg) assert((cond) && (msg))
```

```cpp
// Usage
void HealthComponent::TakeDamage(float amount)
{
    ASSERT_MSG(amount >= 0.0f, "TakeDamage: amount must be positive");
    m_currentHealth -= amount;
    if (m_currentHealth < 0.0f)
        m_currentHealth = 0.0f;
}

TileData& TileMap::GetTile(int x, int y)
{
    ASSERT_MSG(x >= 0 && x < m_width, "GetTile: x out of bounds");
    ASSERT_MSG(y >= 0 && y < m_height, "GetTile: y out of bounds");
    return m_tiles[ToIndex(x, y)];
}
```

### 5.2 `std::optional` — retour qui peut échouer (sans exception)

Équivalent de `T?` (nullable) en C#.

```cpp
#include <optional>

// Retourne un ennemi ou rien si non trouvé
[[nodiscard]] std::optional<EnemyEntity*> FindEnemyById(int id)
{
    for (auto& enemy : m_enemies)
    {
        if (enemy.GetId() == id)
            return &enemy;
    }
    return std::nullopt;  // équivalent de return null en C#
}

// Usage
auto result = world.FindEnemyById(42);
if (result.has_value())
{
    result.value()->TakeDamage(10.0f);
}
```

### 5.3 `[[nodiscard]]` — forcer la vérification des retours

```cpp
// Le compilateur avertit si le retour est ignoré
[[nodiscard]] bool LoadMap(const std::string& path);
[[nodiscard]] std::optional<Texture2D> LoadTextureSafe(const std::string& path);

// ❌ Compilation warning si on écrit juste :
LoadMap("dungeon.json");   // warning : return value ignored

// ✅ Correct :
bool ok = LoadMap("dungeon.json");
ASSERT_MSG(ok, "Failed to load dungeon map");
```

---

## 6. Organisation des fichiers

```
zelda-like/
├── src/
│   ├── core/
│   │   ├── game.h / game.cpp          ← boucle principale, init Raylib
│   │   ├── game_world.h / .cpp        ← conteneur d'entités et de la map
│   │   └── assert.h                   ← macros assert
│   ├── entities/
│   │   ├── entity.h                   ← classe de base abstraite
│   │   ├── living_entity.h / .cpp
│   │   ├── player_entity.h / .cpp
│   │   └── enemy_entity.h / .cpp
│   ├── components/
│   │   ├── health_component.h / .cpp
│   │   ├── collision_component.h / .cpp
│   │   └── sprite_component.h / .cpp
│   ├── states/
│   │   ├── player_state.h             ← interface de base
│   │   ├── idle_state.h / .cpp
│   │   ├── walk_state.h / .cpp
│   │   └── attack_state.h / .cpp
│   ├── map/
│   │   ├── tile_map.h / .cpp
│   │   └── tile_data.h
│   └── main.cpp
├── assets/
│   ├── sprites/
│   ├── tilesets/
│   └── maps/
├── CMakeLists.txt
└── README.md
```

### 6.1 Règle des headers

```cpp
// Chaque .h commence par :
#pragma once  // pas de include guards à la main

// Forward declarations plutôt que #include quand possible
// (réduit les dépendances et les temps de compilation)

// ✅ Dans entity.h — pas besoin d'inclure player_state.h
class PlayerState;  // forward declaration suffit pour un pointeur/référence

// ❌ Éviter dans un header
#include <raylib.h>  // seulement si le type est utilisé directement dans le header
using namespace std; // jamais dans un header
```

### 6.2 CMakeLists.txt de base

```cmake
cmake_minimum_required(VERSION 3.20)
project(ZeldaLike CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # pas de GNU extensions

# Warnings stricts (équivalent de /W4 en MSVC)
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(raylib REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
```

---

## 7. Règles spécifiques Raylib

```cpp
// Les types Raylib (Vector2, Rectangle, Color...) sont des structs C
// → pas de constructeur, initialiser avec des accolades
Vector2 pos = { 0.0f, 0.0f };
Rectangle hitbox = { pos.x, pos.y, 16.0f, 16.0f };
Color tint = WHITE;

// Textures : charger une fois, stocker, décharger à la fin
// ❌ Ne jamais appeler LoadTexture() chaque frame
Texture2D tex = LoadTexture("hero.png");  // une fois en init
// ... utilisation ...
UnloadTexture(tex);  // en fin de vie, pas via destructeur Raylib automatique

// ✅ Wrapper RAII pour gérer la durée de vie automatiquement (comme using en C#)
class ManagedTexture
{
public:
    explicit ManagedTexture(const std::string& path)
        : m_texture(LoadTexture(path.c_str()))
    {
        ASSERT_MSG(m_texture.id != 0, "Failed to load texture");
    }

    ~ManagedTexture() { UnloadTexture(m_texture); }

    // Non-copiable (la texture appartient à cet objet)
    ManagedTexture(const ManagedTexture&) = delete;
    ManagedTexture& operator=(const ManagedTexture&) = delete;

    [[nodiscard]] const Texture2D& Get() const { return m_texture; }

private:
    Texture2D m_texture;
};
```

---

## 8. Récapitulatif — Ce que le LLM doit respecter

| Règle | Valeur |
|---|---|
| Standard | C++17 strict, pas d'extensions compilateur |
| Framework | Raylib (types natifs : Vector2, Rectangle, Color…) |
| Naming classes/méthodes | PascalCase |
| Naming variables/params | camelCase |
| Naming membres privés | `m_` + camelCase |
| Naming constantes | `k` + PascalCase |
| Naming fichiers | snake_case |
| Accolades | Style Allman (accolade sur nouvelle ligne) |
| Indentation | 4 espaces |
| Allocation heap | `std::make_unique<>` uniquement, jamais `new` brut |
| Exceptions | Interdites (`-fno-exceptions`) |
| Gestion d'erreurs | `assert` en debug, `std::optional` pour les retours faillibles |
| Return values importants | `[[nodiscard]]` obligatoire |
| Headers | `#pragma once`, forward declarations privilégiées |
| `using namespace std` | Interdit dans les headers, déconseillé partout |
| Copies d'entités | `= delete` sur copy constructor et copy assignment |
| Conteneur par défaut | `std::vector<>` avec `reserve()` si taille connue |
| Allocation en hot path | Interdite — pré-allouer ou réutiliser |