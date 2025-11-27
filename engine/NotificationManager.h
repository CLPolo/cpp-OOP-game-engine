#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace CMPUT350 {

class GameObject;

class NotificationManager
{
public:
    NotificationManager() = default;
    ~NotificationManager() = default;

    // Register an object to receive notifications for a specific key
    void Register(std::weak_ptr<GameObject> object, const std::string& key);

    // Unregister an object from receiving notifications for a specific key
    void Unregister(std::weak_ptr<GameObject> object, const std::string& key);

    // Send a notification to all objects registered for this message
    void Notify(const std::string& message);

    // Helper function to clean up expired weak_ptr references
    void CleanupExpired();
    
private:
    // Map of message keys to lists of objects listening for those messages
    std::unordered_map<std::string, std::vector<std::weak_ptr<GameObject>>> mListeners;

};

}

#endif // NOTIFICATIONMANAGER_H
