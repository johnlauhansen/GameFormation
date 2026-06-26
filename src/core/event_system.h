#pragma once
#include <functional>
#include <vector>
#include <raylib.h>

/*
 * Événement déclenché à la destruction d'une caisse en bois.
 */
struct CrateDestroyedEvent
{
    Vector2 position; /* Coordonnées mondiales où la caisse a été détruite */
};

/* Type de callback à enregistrer pour écouter la destruction des caisses */
using CrateDestroyedCallback = std::function<void(const CrateDestroyedEvent&)>;

/*
 * Bus d'événements central (Observer Pattern).
 * Permet un découplage total entre les quêtes, les décors, et le système de combat.
 */
class EventSystem
{
public:
    /*
     * Enregistre un écouteur de destruction de caisses.
     * @param[in] callback Expression lambda ou pointeur de fonction à appeler.
     */
    static void SubscribeToCrateDestroyed(CrateDestroyedCallback callback);

    /*
     * Diffuse un événement de destruction de caisses à tous les écouteurs abonnés.
     * @param[in] event Contient les informations associées au sinistre.
     */
    static void PublishCrateDestroyed(const CrateDestroyedEvent& event);

    /*
     * Vide la liste des abonnés (à appeler lors du nettoyage ou du rechargement de niveau).
     */
    static void Clear();

private:
    static std::vector<CrateDestroyedCallback> s_crateDestroyedListeners;
};
