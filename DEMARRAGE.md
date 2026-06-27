# Démarrage

Ce document fournit des instructions détaillées pour démarrer et configurer votre projet.

## Prérequis

Avant de commencer, assurez-vous d'avoir les éléments suivants installés :

- [CMake](https://cmake.org/)
- [GCC](https://gcc.gnu.org/) ou [Clang](https://clang.llvm.org/)
- [Python](https://www.python.org/)

## Installation

1. **Clonez le dépôt** :
   ```sh
   git clone https://github.com/votre-nom-de-repo/votre-projet.git
   cd votre-projet
   ```

2. **Créez un environnement virtuel (optionnel mais recommandé)** :
   ```sh
   python -m venv venv
   source venv/bin/activate  # Sous Windows utilisez `venv\Scripts\activate`
   ```

3. **Installez les dépendances** :
   ```sh
   pip install -r requirements.txt
   ```

## Compilation

1. **Générez le projet avec CMake** :
   ```sh
   mkdir build
   cd build
   cmake ..
   make  # ou `ninja` si vous utilisez Ninja comme générateur
   ```

2. **Exécutez le jeu** :
   ```sh
   ./game
   ```

## Configuration

Vous pouvez personnaliser certaines configurations en modifiant les fichiers dans le répertoire `config`.

## Fichiers importants

- **GEMINI.md** : Ce fichier fournit des informations détaillées sur la structure et le contenu du projet.
- **DEMARRAGE.md** : Ce document fournit des instructions détaillées pour démarrer et configurer votre projet.
- **markdown/** : Ce dossier contient tous les fichiers Markdown à prendre en compte pour le projet.

## Résolution de problèmes

Si vous rencontrez des problèmes, consultez la section [Problèmes courants](#problèmes-courants) ou contactez le support.

### Problèmes courants

- **Erreur de compilation** : Assurez-vous que toutes les dépendances sont correctement installées.
- **Problème d'exécution** : Vérifiez les permissions du fichier exécutable et assurez-vous qu'il n'y a pas de problèmes de compatibilité.

Si vous avez des questions supplémentaires ou si vous rencontrez des problèmes, n'hésitez pas à contacter le support.
