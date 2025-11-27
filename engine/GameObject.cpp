#include "GameObject.h"

namespace CMPUT350 {
GameObject::GameObject() {}
// Update functions
void GameObject::Update(GameContext *context) { return; }
void GameObject::LateUpdate(GameContext *context) { return; }

// Redering functions
void GameObject::RenderBackground(GameContext *contextrender) { return; }
void GameObject::RenderForeground(GameContext *contextrender) { return; }

// Key press
bool GameObject::HandleKeyEvent(GameContext *context, char key) { return false; }

// Alive functions
bool GameObject::IsAlive() { return mAlive; }
void GameObject::Kill() { mAlive = false; }

// Notification function
void GameObject::ReceiveNotification(const std::string& message) { return; }

}