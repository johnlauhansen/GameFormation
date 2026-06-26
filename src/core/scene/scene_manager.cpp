#include "scene_manager.h"
#include "scene.h"

SceneManager::SceneManager()
    : m_currentScene(nullptr)
{
}

SceneManager::~SceneManager()
{
    if (m_currentScene != nullptr)
    {
        m_currentScene->OnExit();
    }
}

void SceneManager::RegisterScene(const std::string& name, std::unique_ptr<Scene> scene)
{
    m_scenes[name] = std::move(scene);
}

void SceneManager::ChangeScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it != m_scenes.end())
    {
        if (m_currentScene != nullptr)
        {
            m_currentScene->OnExit();
        }
        
        m_currentScene = it->second.get();
        m_currentScene->OnEnter();
    }
}

void SceneManager::Update(float deltaTime)
{
    if (m_currentScene != nullptr)
    {
        m_currentScene->Update(deltaTime, *this);
    }
}

void SceneManager::Draw() const
{
    if (m_currentScene != nullptr)
    {
        m_currentScene->Draw();
    }
}

Scene* SceneManager::GetCurrentScene() const
{
    return m_currentScene;
}
