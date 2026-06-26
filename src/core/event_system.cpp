#include "event_system.h"

std::vector<CrateDestroyedCallback> EventSystem::s_crateDestroyedListeners;

void EventSystem::SubscribeToCrateDestroyed(CrateDestroyedCallback callback)
{
    s_crateDestroyedListeners.push_back(std::move(callback));
}

void EventSystem::PublishCrateDestroyed(const CrateDestroyedEvent& event)
{
    for (const auto& listener : s_crateDestroyedListeners)
    {
        listener(event);
    }
}

void EventSystem::Clear()
{
    s_crateDestroyedListeners.clear();
}
