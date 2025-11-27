#include "NotificationManager.h"
#include "GameObject.h"
#include <algorithm>

namespace CMPUT350 {

void NotificationManager::Register(std::weak_ptr<GameObject> object, const std::string& key) 
{   /**
     * @brief Registers a game object to receive a notification.
     * @param object Weak pointer to the notification recipient.
     * @param key The notification to send to the recipient.
     *///TODO: comment out the rest of NoteMan
    mListeners[key].push_back(object);
}

void NotificationManager::Unregister(std::weak_ptr<GameObject> object, const std::string& key)
{
    // Find the key in the map
    auto it = mListeners.find(key); // find returns an iterator
    if (it == mListeners.end())
    {
        return; // Key not found, nothing to unregister
    }

    // Get the vector of listeners for this key
    // see https://stackoverflow.com/questions/15451287/what-does-iterator-second-mean
    auto& listeners = it->second; 

    // Try to lock the weak_ptr to compare
    // see https://en.cppreference.com/w/cpp/memory/weak_ptr/lock
    auto objPtr = object.lock();
    if (!objPtr)
    {
        return; // Object already expired
    }

    // Remove the object from the listeners
    listeners.erase(    // see https://stackoverflow.com/questions/39019806/using-erase-remove-if-idiom
        std::remove_if( // and https://en.cppreference.com/w/cpp/algorithm/remove.html
            listeners.begin(),
            listeners.end(),
            [&objPtr](const std::weak_ptr<GameObject>& weakObj) {
                auto ptr = weakObj.lock();// see https://stackoverflow.com/questions/15451287/what-does-iterator-second-mean
                return !ptr || ptr == objPtr;
            }),
        listeners.end()
    );

    // If no more listeners for this key, remove the key from the map
    if (listeners.empty())
    {
        mListeners.erase(it);
    }
}

void NotificationManager::Notify(const std::string& message)
{
    // Find the key in the map
    auto it = mListeners.find(message);
    if (it == mListeners.end())
    {
        return; // No listeners for this message
    }

    // Get the vector of listeners for this key
    auto& listeners = it->second;

    // Deliver notifications to all valid listeners
    // Iterate with index to avoid iterator invalidation
    for (size_t i = 0; i < listeners.size(); )
    {
        auto objPtr = listeners[i].lock();
        if (objPtr)
        {
            // Object is still alive, send notification
            objPtr->ReceiveNotification(message);
            ++i;
        }
        else
        {
            // Object expired, remove from list
            listeners.erase(listeners.begin() + i);
            // Don't increment i, as we just removed an element
        }
    }

    // If no more listeners after cleanup, remove the key
    if (listeners.empty())
    {
        mListeners.erase(it);
    }
}

void NotificationManager::CleanupExpired()
{
    for (auto it = mListeners.begin(); it != mListeners.end(); )
    {
        auto& listeners = it->second;

        // Remove expired weak_ptrs from the vector
        listeners.erase(
            std::remove_if(
                listeners.begin(),
                listeners.end(),
                [](const std::weak_ptr<GameObject>& weakObj) {
                    return weakObj.expired();
                }),
            listeners.end()
        );

        // If no listeners remain, erase this key entirely
        if (listeners.empty())
        {
            it = mListeners.erase(it);  // erase returns the next valid iterator
        }
        else
        {
            ++it;
        }
    }
}

}
