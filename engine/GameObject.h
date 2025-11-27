#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

namespace CMPUT350
{
class GameObject;
}

#include <vector>
#include <string>
#include "GameContext.h"

namespace CMPUT350
{

class GameObject
{
public:
    GameObject();
    virtual ~GameObject() = default;
    virtual void Update(GameContext *context);
    virtual void LateUpdate(GameContext *context);
    virtual void RenderBackground(GameContext *context);
    virtual void RenderForeground(GameContext *context);
    virtual bool HandleKeyEvent(GameContext *context, char key);
    virtual bool IsAlive();
    virtual void Kill();
    virtual void ReceiveNotification(const std::string& message);

private:
    bool mAlive = true;
};

}

#endif // GAMEOBJECT_H
