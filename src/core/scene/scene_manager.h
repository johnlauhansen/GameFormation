#pragma once
#include <memory>
#include <string>
#include <unordered_map>

class Scene;

/*
 * Gestionnaire de scènes et d'écrans de jeu (State Machine / FSM d'états d'écrans).
 * Gère le dictionnaire des scènes du moteur (Titre, Gameplay, Options, Forge) et orchestre les transitions.
 */
class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    /*
     * Enregistre une scène nommée dans le dictionnaire pour un usage futur.
     * @param[in] name Nom d'identification unique de la scène (ex: "Gameplay").
     * @param[in] scene Pointeur unique vers l'instance de la scène.
     */
    void RegisterScene(const std::string& name, std::unique_ptr<Scene> scene);

    /*
     * Désactive la scène courante (OnExit) et démarre la scène cible (OnEnter).
     * @param[in] name Identifiant unique de la scène vers laquelle basculer.
     */
    void ChangeScene(const std::string& name);

    /*
     * Met à jour la logique interne de la scène active de la frame.
     * @param[in] deltaTime Temps écoulé depuis la dernière frame.
     */
    void Update(float deltaTime);

    /*
     * Dessine l'ensemble des éléments graphiques de la scène active.
     */
    void Draw() const;

    /*
     * Récupère un pointeur vers la scène active.
     * @return Pointeur brut vers la scène en cours d'exécution.
     */
    [[nodiscard]] Scene* GetCurrentScene() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_currentScene;
};
