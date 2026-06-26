#pragma once
#include <functional>
#include <vector>
#include <raylib.h>

struct CrateDestroyedEvent
{
    Vector2 position;
};

using CrateDestroyedCallback = std::function<void(const CrateDestroyedEvent&)>;

class EventSystem
{
public:
    static void SubscribeToCrateDestroyed(CrateDestroyedCallback callback);
    static void PublishCrateDestroyed(const CrateDestroyedEvent& event);
    static void Clear();

private:
    static std::vector<CrateDestroyedCallback> s_crateDestroyedListeners;
};
