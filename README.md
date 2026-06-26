# 🎮 gameFormation - C++ Game Development Starter

Bienvenue dans votre environnement de remise à niveau en C/C++ ! En tant que développeur C# chevronné (10 ans d'XP), vous allez rapidement retrouver vos repères. C++ moderne (C++17/20) partage énormément de concepts familiers avec le C# moderne, tout en vous donnant un contrôle total sur la mémoire.

Ce projet est configuré avec **Raylib** (via CMake), une bibliothèque de jeu extrêmement légère, performante et propre, parfaite pour faire des jeux 2D/3D sans la lourdeur d'un moteur comme Unreal ou Unity.

---

## 🚀 1. Le Secret pour switcher facilement : Les Profils VS Code

Pour éviter que VS Code ne devienne lourd en activant simultanément les serveurs de langage pour **C/C++ (OmniSharp/Roslyn)**, **C#** et **Java**, la solution ultime consiste à utiliser les **Profils VS Code**.

### Comment créer vos Profils :
1. Cliquez sur la **Roue crantée** (en bas à gauche) -> **Profiles** -> **Create Profile...**
2. Créez un profil nommé **"C++ Game Dev"** :
   - Activez les extensions : `C/C++`, `C/C++ Extension Pack`, `CMake`, `CMake Tools`.
   - Désactivez toutes les extensions C# et Java dans ce profil.
3. Créez un profil nommé **"C# .NET"** :
   - Activez les extensions : `C#`, `C# Dev Kit`.
   - Désactivez C++ et Java.
4. Créez un profil nommé **"Java"** :
   - Activez : `Extension Pack for Java`.
   - Désactivez C++ et C#.

**Pour switcher :** Un simple clic sur l'icône de profil (en bas à gauche) vous permet de basculer instantanément de profil. VS Code rechargera uniquement les extensions nécessaires, gardant votre éditeur ultra-rapide et exempt de conflits !

---

## 🛠️ 2. Prérequis pour compiler le C++

Puisque vous êtes développeur C#, vous avez très probablement **Visual Studio (Community/Professional)** installé sur votre machine Windows. Cela signifie que vous possédez déjà le compilateur officiel de Microsoft (**MSVC / cl.exe**).

### A. Vérifier ou installer le compilateur :
* Si vous n'avez pas de compilateur C++, ouvrez le **Visual Studio Installer**, cliquez sur *Modifier* sur votre instance de Visual Studio, et cochez la case **"Développement de jeux en C++"** ou **"Développement Desktop en C++"**. Cela installera MSVC et CMake si ce n'est pas déjà fait.

### B. Extensions VS Code recommandées :
Ouvrez ce dossier dans VS Code. Il vous proposera automatiquement d'installer les extensions recommandées (définies dans `.vscode/extensions.json`) :
1. **C/C++ Extension Pack** (Microsoft)
2. **CMake Tools** (Microsoft)

---

## 🔨 3. Configurer, Compiler et Lancer le Jeu

Grâce à **CMake** et sa fonctionnalité `FetchContent` configurée dans ce projet, **vous n'avez rien à installer manuellement !** Raylib sera téléchargé, configuré et compilé statiquement de manière transparente lors du premier build.

### Étape par étape avec Visual Studio Code :
1. **Ouvrir le dossier dans VS Code** : `C:\GIT\Formation\gameFormation`
2. **Sélectionner un "Kit" (Compilateur)** :
   - En bas dans la barre d'état de VS Code (ou via `Ctrl+Shift+P` -> `CMake: Select a Kit`), choisissez votre compilateur.
   - Sélectionnez par exemple **Visual Studio Community 2022 Release - amd64** (ou similaire).
3. **Sélectionner la cible active** :
   - Le projet définit deux cibles exécutables :
     *   `gameFormation` : Le jeu principal (village RPG, quêtes, marchands et combats).
     *   `gameplay_sandbox` : Le bac à sable expérimental (moteur physique de boomerang).
   - Dans la barre d'état tout en bas de VS Code, cliquez sur l'icône de cible à côté de "Build" (ou via `Ctrl+Shift+P` -> `CMake: Set Debug Target`) et choisissez votre cible active.
4. **Compiler** :
   - Cliquez sur **Build** dans la barre d'état tout en bas (ou faites `F7` ou via `Ctrl+Shift+P` -> `CMake: Build`).
5. **Lancer ou déboguer** :
   - **Lancement direct (sans débogueur)** : Cliquez sur le bouton **Play (Launch)** dans la barre d'état tout en bas (ou faites `Ctrl+F5`).
   - **Débogage pas à pas (avec débogueur)** : Cliquez sur l'icône de **Coccinelle (Debug)** dans la barre d'état (ou faites `F5` pour compiler et démarrer une session de débogage MSVC complète).

---

### Étape par étape avec Visual Studio (IDEE Complète) :
Comme vous êtes développeur C#, vous pouvez également ouvrir et exécuter ce projet directement dans l'IDEE de bureau de Visual Studio (sans aucun fichier `.sln` ou `.csproj` !) grâce au support CMake natif :
1. **Ouvrir le projet** :
   - Ouvrez **Visual Studio**, allez dans **Fichier** -> **Ouvrir** -> **Dossier...** (ou *Open Local Folder*).
   - Sélectionnez le dossier racine du projet `C:\GIT\Formation\gameFormation`.
2. **Configuration automatique** :
   - Visual Studio détecte immédiatement le fichier `CMakeLists.txt` racine et démarre la génération du cache CMake (visible dans la fenêtre de sortie). Il va télécharger automatiquement Raylib et nlohmann/json.
3. **Sélectionner la cible de démarrage** :
   - Dans la barre d'outils supérieure de Visual Studio, à côté du bouton de démarrage vert (Play), cliquez sur le menu déroulant de sélection d'élément de démarrage (Startup Item).
   - Choisissez l'exécutable souhaité : **`gameFormation.exe`** ou **`gameplay_sandbox.exe`**.
4. **Lancer le jeu** :
   - Cliquez sur le bouton de démarrage vert **"Démarrer"** (ou faites `F5` pour compiler et lancer en mode débogage, ou `Ctrl+F5` pour lancer sans débogage).
   - Visual Studio compilera les fichiers modifiés et affichera instantanément la fenêtre OpenGL du jeu !

---

## 🧠 Repères C# vs C++ Moderne pour vous aider :

| Concept | En C# | En C++ Moderne (C++17) | Note |
| :--- | :--- | :--- | :--- |
| **Gestion mémoire** | Garbage Collector | RAII & Smart Pointers (`std::unique_ptr`, `std::shared_ptr`) | Plus besoin de faire de `delete` manuel ! Le destructeur s'occupe de tout à la sortie du scope. |
| **Passage d'arguments** | Par référence (`ref` / `out`) | Références constantes (`const Type&`) | Évite la copie d'objets lourds tout en garantissant l'immutabilité. |
| **Inclusions** | `using Namespace;` | `#include <header>` | C++ utilise un système d'inclusion physique de fichiers d'en-tête (headers) avant compilation. |
| **Types dynamiques** | `var` | `auto` | Résolution de type statique à la compilation. |
| **Valeur nulle** | `null` | `nullptr` | Plus robuste et typé que l'ancien `NULL` hérité du C. |
| **Collections** | `List<T>`, `Dictionary<K, V>` | `std::vector<T>`, `std::unordered_map<K, V>` | `std::vector` est le conteneur par défaut à privilégier absolument. |
