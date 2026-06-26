# 🏛️ Bonnes Pratiques d'Architecture - Action-RPG 2D (Zelda-like)

Ce document récapitule les standards et recommandations architecturales pour le développement et l'évolution de **gameFormation**. Ces règles visent à maintenir un couplage faible, une extensibilité maximale et une haute performance en C++17.

---

## ⚔️ 1. Gestion des Entités : La Composition (KISS / SOLID)

Pour éviter les hiérarchies d'héritage rigides et profondes (ex. `Entity -> Character -> Enemy -> Octorok`), nous privilégions la **composition légère** :

*   **Principe de Responsabilité Unique (SRP)** : Une entité (comme le joueur ou un ennemi) délègue ses fonctionnalités spécifiques à des classes ou structures de données dédiées.
*   **Séparation des Données et de la Logique** :
    *   Les attributs physiques d'un objet (Hitbox, Position, Vélocité, Collision) doivent être manipulés via des fonctions pures ou des modules spécialisés.
    *   Exemple : Notre gestion de l'inventaire (`Inventory`) qui est un composant de `Player`, et le `HUD` qui gère le dessin de façon totalement isolée.
*   **Évolutivité vers l'ECS** : Si le nombre d'entités interactives augmente considérablement, l'architecture doit faciliter la transition vers un système de composants complet (Entity Component System) où les entités ne sont que des identifiants et les systèmes gèrent la logique globale.

---

## 📡 2. Découplage Systémique par Événements (Observer Pattern)

Dans un Zelda-like, de nombreuses actions déclenchent des réactions en chaîne (ex. vaincre tous les monstres d'une salle ouvre la porte ou fait apparaître un coffre).

*   **Interdiction du Couplage Fort** : Une classe d'ennemi ne doit jamais posséder un pointeur direct vers une porte ou un coffre spécifique pour interagir avec eux.
*   **Messagerie Événementielle (Event Broker)** :
    *   Les entités publient des événements à un courtier central (ex: `EnemyKilledEvent`).
    *   Les gestionnaires de niveau (`DungeonManager` ou `GameWorld`) s'abonnent à ces événements et orchestrent les conséquences sur la carte.
    *   Cela garantit que les monstres, les obstacles et les récompenses peuvent être développés, testés et modifiés de manière totalement indépendante.

---

## 🗺️ 3. Structure du Monde et Physique Locale (Grid Partitioning)

La performance d'un Action-RPG repose sur une gestion efficace de l'espace de jeu.

*   **Partitionnement par Grille (Grid Partitioning)** :
    *   Le monde est découpé en tuiles (`TileMap`) de dimensions fixes (ex. `kTileSize = 32`).
    *   La détection de collision solide ne doit tester **que** les tuiles adjacentes à la boîte englobante (AABB) de l'entité active. La complexité doit rester en $O(1)$ par rapport à la taille totale de la carte.
*   **Gestion des Écrans (Transitions de Salles)** :
    *   Inspiré des classiques GameBoy/NES, le monde peut être découpé en salles distinctes.
    *   Figer la logique des entités situées en dehors de l'écran actif permet de libérer du temps CPU et de simplifier l'écriture de l'intelligence artificielle des ennemis (qui n'ont pas à gérer le hors-champ).

---

## 💾 4. Centralisation des Ressources (Asset Manager / Service Locator)

Les accès disque sont lents et provoquent des saccades d'affichage (*stuttering*).

*   **Pas de chargement à la volée** : Ne chargez jamais une texture, une police ou un son directement dans la boucle de rendu ou d'update (ex: dans un `Draw()` ou `Update()`).
*   **Asset Manager Cache** :
    *   Toutes les ressources graphiques, sonores et de données de niveau doivent être chargées lors de phases dédiées (chargement de niveau, démarrage).
    *   Elles sont stockées dans un cache centralisé (`AssetManager`) indexé par des identifiants uniques (`std::string` ou `enum`).
    *   Les entités ne stockent que des références ou des pointeurs non-propriétaires vers ces ressources partagées.

---

## ⚙️ 5. Cycle de Vie et États Bloquants (FSM / Context Freeze)

La boucle principale doit s'adapter à des contextes de jeu radicalement différents (Jeu actif, Menu de forge/pause, Dialogues, Transitions).

*   **Machine à États Finis (FSM)** :
    *   Le jeu principal (`Game`) utilise une machine à états claire pour basculer entre les écrans principaux (Menu, Gameplay, Options).
    *   Les entités comme le joueur (`Player`) utilisent également une FSM locale (`PlayerState` : *Idle*, *Walking*, *Attacking*, *HitStun*) pour éviter les branches conditionnelles complexes et incohérentes.
*   **Figeage Contextuel (Freeze)** :
    *   Lorsqu'un menu de dialogue ou le Blacksmith Forge est ouvert, la physique et l'update du `GameWorld` sont mis en pause (figés), tandis que le rendu continue d'afficher le monde en arrière-plan.
    *   Ceci est réalisé proprement en séparant l'appel des fonctions d'Update du monde de celles de l'interface utilisateur.

---

## 📊 6. Conception Pilotée par les Données (Data-Driven Design)

L'équilibrage et le contenu du jeu doivent être découplés du code compilé.

*   **Formats Ouverts (JSON)** :
    *   Les cartes (Tiled JSON), les statistiques des armes, les configurations des ennemis et les dialogues doivent être stockés sous forme de fichiers de données externes.
    *   L'ajout de contenu ne doit pas nécessiter de recompilation du moteur de jeu.
*   **Validation des Données** :
    *   Le parseur (ex: `MapLoader`) doit valider la structure des fichiers importés et appliquer des valeurs par défaut sécurisées en cas de fichier corrompu pour éviter les crashs à l'exécution.
