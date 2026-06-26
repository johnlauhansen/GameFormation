#pragma once
#include <memory>
#include <string>
#include <unordered_map>

class Scene;

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    /* Enregistre une scène dans le dictionnaire */
    void RegisterScene(const std::string& name, std::unique_ptr<Scene> scene);

    /* Change la scène active et déclenche OnExit() / OnEnter() */
    void ChangeScene(const std::string& name);

    /* Mise à jour et rendu de la scène courante */
    void Update(float deltaTime);
    void Draw() const;

    [[nodiscard]] Scene* GetCurrentScene() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_currentScene;
};
