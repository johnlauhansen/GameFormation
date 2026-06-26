# 🤖 DIRECTIVE DE CONTEXTE POUR ASSISTANT IA (LLM_Context_Directive)

Ce document sert de **spécification de contexte de démarrage** pour tout Assistant IA (LLM) reprenant le développement de **gameFormation**. Il contient l'état de l'architecture, la charte de codage, les mécanismes de sécurité C++ et les recettes de développement pour étendre le moteur de jeu de façon 100% cohérente.

---

## 🎯 1. Identité du Projet & Pile Technique
*   **Objectif** : Un jeu Action-RPG 2D style Zelda rétro développé en C++ Moderne.
*   **Cible technique** : **C++17 Strict** (MSVC / GCC / Clang), compilé via CMake.
*   **Bibliothèques** : **Raylib 5.0** (moteur de rendu OpenGL) et **nlohmann/json 3.11.3** (parser de cartes).
*   **Zéro Exception** : Compilation avec `-fno-exceptions` (ou `/EHsc` sécurisé MSVC). Toute erreur critique doit être levée via `assert()` ou gérée via des retours optionnels (`std::optional`).
*   **Sécurité mémoire** : Intégration de l'**AddressSanitizer (ASan)** dans `CMakeLists.txt`. Tout accès mémoire invalide ou fuite doit être immédiatement corrigé.

---

## 🏛️ 2. Architecture Modulaire à Responsabilité Unique (SRP)
Le moteur a été découpé pour éliminer les classes omniscientes ("God Classes"). Il est divisé en sous-systèmes spécialisés :

### A. Contrôleurs de Scène (UI & Pile d'Écrans)
*   **`SceneManager`** : Automate à états (FSM) gérant la scène active (`TitleScene`, `GameplayScene`, `OptionsScene`, `InventoryScene`).
*   **`Scene`** : Interface polymorphique pure avec cycle de vie `OnEnter()`, `OnExit()`, `Update()`, `Draw()`.

### B. Gestionnaires du Monde de Jeu
*   **`Game`** : Point d'entrée de l'application, détient le `SceneManager` et la boucle de rendu principale à l'échelle (Letterbox 4:3).
*   **`GameWorld`** : Orchestrateur exclusif du gameplay actif, détient la carte physique (`TileMap`), le joueur (`Player`), le contrôleur lissé de caméra (`CameraController`) et le gestionnaire d'acteurs (`EntityManager`).

### C. Gestion des Entités & Optimisation Spatiale
*   **`EntityManager`** : Propriétaire exclusif du cycle de vie des entités (`Npc`, `Enemy`, `Destructible`, `GroundPickup`, `Portal`).
*   **`SpatialGrid` (Partitionnement Spatial Broadphase)** : Indexe les destructibles par cellules géographiques de $160\text{px}\times160\text{px}$. Abaisse la complexité de détection de collisions physiques de $\mathcal{O}(N)$ à $\mathcal{O}(1)$.

### D. Systèmes Applicatifs purs (Namespaces de fonctions)
*   **`PhysicsSystem`** : Gère la physique de glissement AABB sur deux axes (X, Y) du joueur contre la grille et les destructibles indexés spatialement.
*   **`CombatSystem`** : Résout les attaques à l'épée, les trajectoires de retour du boomerang, les dégâts de contact des monstres, et l'invulnérabilité du joueur (iframes).
*   **`DialogueSystem`** : Gère la transition de dialogue et le menu de boutique marchand en espace écran hors caméra.

### E. Services Généraux & Données
*   **`Database` (Data-Driven)** : Parse les fichiers JSON `enemies.json` et `dialogues.json` au démarrage. Tout comportement ou statistique est lu de façon externe (Gabarit).
*   **`EventSystem` (Observer Pattern)** : Bus d'événements global découplant les systèmes. (Ex: Le combat détruit une caisse, diffuse un événement, le PNJ de quête s'y abonne et incrémente son compteur de manière autonome).

---

## 📜 3. Charte de Codage & Formats (Strict)

### A. Indentation & Accolades
*   **Style Allman obligatoire** : Accolade ouvrante sur sa propre ligne.
*   **Indentation** : **4 espaces uniquement**, jamais de tabulations.
*   **Longueur de ligne** : **120 caractères** maximum.

```cpp
if (condition)
{
    DoSomething();
}
```

### B. Conventions de Nommage
*   **Classes, Structures, Énumérations** : `PascalCase` (ex: `Player`, `SpatialGrid`, `EnemyState`).
*   **Méthodes / Fonctions** : `PascalCase` (ex: `Update()`, `CheckCollision()`).
*   **Variables locales / Paramètres** : `camelCase` (ex: `deltaTime`, `targetPos`).
*   **Membres privés** : Préfixe `m_` suivi de `camelCase` (ex: `m_position`, `m_speed`).
*   **Constantes / constexpr** : Préfixe `k` suivi de `PascalCase` (ex: `kTileSize`).
*   **Fichiers** : `snake_case` (ex: `entity_manager.h`, `spatial_grid.cpp`).

### C. Commentaires et Auto-Documentation
*   **Headers (`.h`)** : Commentaires au format **Doxygen** obligatoires pour chaque classe et déclaration publique (expliquant les `@param` et `@return`).
*   **Sources (`.cpp`)** : Commentaires internes expliquant le **"pourquoi"** de l'opération et non le "quoi".

---

## 🛠️ 4. Recettes de Développement (Comment Étendre le Moteur)

### A. Comment ajouter un nouvel Ennemi ?
1.  **Base de données** : Ajoutez sa fiche technique dans `assets/databases/enemies.json` avec ses PV, dégâts, vitesse, rayon de détection et le nom de son comportement (ex: `"behavior": "Squelette"`).
2.  **Héritage IA** : Créez la classe dérivée `SqueletteBehavior : public EnemyBehavior` dans `src/core/entity/enemy_behavior.h`.
3.  **Implémentation** : Codez ses fonctions virtuelles `Update()` et `Draw()` dans `src/core/entity/enemy_behavior.cpp`.
4.  **Enregistrement** : Modifiez le constructeur d'`Enemy::Enemy` dans `enemy.cpp` pour instancier votre comportement s'il correspond au nom du JSON.

### B. Comment ajouter une nouvelle Quête ?
1.  **Événement** : Si la quête dépend d'une nouvelle action (ex: tuer un ennemi), déclarez l'événement associé (ex: `struct EnemyKilledEvent`) dans `event_system.h` ainsi que son callback d'abonnement. Diffusez-le lors du décès dans `CombatSystem`.
2.  **PNJ** : Modifiez `Npc::ConfigureQuest()` dans `npc.cpp` pour vous abonner à ce nouvel événement via l'`EventSystem` et incrémentez le compteur de quête si elle est active.

### C. Comment ajouter une nouvelle Scène ?
1.  **Classe** : Créez `MaScene : public Scene` dans `src/core/scene/`.
2.  **Enregistrement** : Déclarez-la et enregistrez-la dans le constructeur de la classe `Game` dans `game.cpp` via `m_sceneManager.RegisterScene()`.
3.  **Transition** : Déclenchez la transition depuis une autre scène via `manager.ChangeScene("MaScene")`.

---

## 🏛️ 5. Historique du Projet & Évolutions Majeures (Chronique Git)
Pour guider un autre LLM sur le "cheminement d'esprit" de cette architecture, voici la chronologie logique des refactorings successifs appliqués sur la branche `develop` :

### Étape 1 : Le Découplage de base (`EntityManager` & `CameraController`)
*   **Problème initial** : `GameWorld` était une God Class détenant tous les vecteurs d'entités, gérant à la fois leur dessin, leur mise à jour, la détection physique brute, et la caméra.
*   **Résolution** : Extraction de `EntityManager` (gestion exclusive des acteurs) et de `CameraController` (lissage par interpolation linéaire de la caméra).

### Étape 2 : Le Bestiaire Flexible (IA Polymorphique - Strategy Pattern)
*   **Problème initial** : La classe `Enemy` contenait de multiples `switch(m_type)` et des conditions d'IA complexes polluant les méthodes de base.
*   **Résolution** : Introduction de l'interface `EnemyBehavior` et implémentation de classes dérivées (`SlimeBehavior`, `OctorokBehavior`, `MoblinBehavior`). La classe `Enemy` délègue sa boucle logique et son dessin de façon polymorphique.

### Étape 3 : Sémantique de Déplacement & Sécurité Mémoire C++ (MSVC / ASan)
*   **Bug d'invalidation de vecteur résolu** : Lors des transitions de portes/bâtiments, appeler `LoadMap()` dans la boucle de collision du joueur libérait le vecteur `m_portals`, transformant la référence de portail active en pointeur d'adresse libéré (*dangling pointer*), lisant ainsi un spawn de repli corrompu `{0, 0}` (mur solide du coin haut gauche). Résolu en mettant en cache les valeurs de transitions localement sur la pile de la fonction *avant* de charger la carte.
*   **Résolution de type incomplet (C2338 / C2665)** : L'utilisation de pointeurs intelligents uniques (`std::unique_ptr`) sur une classe forward-déclarée (`EnemyBehavior`) imposait de déporter explicitement le destructeur, le constructeur de déplacement, et l'opérateur d'affectation par déplacement d'`Enemy` de façon hors ligne (out-of-line) dans `enemy.cpp` pour que le compilateur MSVC dispose d'un type complet lors des opérations sur les vecteurs d'entités.

### Étape 4 : Le Pivot Data-Driven (Gabarits & Tiled Custom Properties)
*   **Résolution** : Migration des statistiques physiques fixes des monstres et des dialogues d'accueil des PNJs vers les fichiers JSON globaux `enemies.json` et `dialogues.json` gérés par la base de données globale `Database`. 
*   **Portails Liés** : Extension des propriétés personnalisées Tiled (`portal_id` et `target_portal_id`) pour lier dynamiquement les entrées/sorties de bâtiments par ID (les portes) au lieu de hardcoder des coordonnées de pixels.

### Étape 5 : Découplage d'Éléments par Bus d'Événements (`EventSystem` - Observer)
*   **Problème initial** : Le combat de destruction de caisses imposait de boucler manuellement sur l'ensemble des PNJs du monde pour incrémenter les quêtes actives (couplage fort).
*   **Résolution** : Établissement du bus de messagerie statique `EventSystem`. Le combat publie l'événement de destruction de caisses, le PNJ s'y abonne de manière autonome via une lambda expression capturante.

### Étape 6 : Automate à états d'Écrans (`SceneManager`)
*   **Problème initial** : La classe d'entrée de jeu `Game` détenait de gigantesques sous-fonctions de dessin d'écrans et une machine à états rudimentaire.
*   **Résolution** : Création d'une structure de scènes polymorphiques (`TitleScene`, `GameplayScene`, `OptionsScene`, `InventoryScene`) gérées par un `SceneManager` léger.

### Étape 7 : L'Indexation Spatiale ($\mathcal{O}(1)$ Collisions via `SpatialGrid`)
*   **Problème initial** : La détection de collisions physiques glissantes et les rectangles d'attaques à l'épée effectuaient des recherches linéaires sur l'ensemble de la liste des destructibles d'un niveau ($\mathcal{O}(N)$), impactant les performances sur les cartes géantes.
*   **Résolution** : Implémentation d'une grille de partitionnement spatial par cellule (`SpatialGrid`). Les requêtes physiques d'attaques ou de glissement demandent désormais les "Nearby" destructibles, limitant l'itération à un temps constant très performant ($\mathcal{O}(1)$).

### Étape 8 : Organisation Structurée des Fichiers
*   **Résolution** : Rangement physique des 40+ fichiers C++ de la racine du noyau vers les répertoires `/player`, `/entity`, `/system`, `/scene` et `/ui`. La compatibilité des directives d'inclusion `#include` locales a été préservée sans modification de fichiers en modifiant la variable `target_include_directories` dans `CMakeLists.txt`.

