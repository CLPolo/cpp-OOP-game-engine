#ifndef GAMECONTEXT_H
#define GAMECONTEXT_H


#include "EngineView.h"
#include "DrawContext.h"
#include "GameObject.h"
#include "NotificationManager.h"

#include <memory>

namespace CMPUT350 {

class GameContext {
    public:
        EngineView *EngineContext;
        DrawContext *ScreenContext;
        DrawContext *GUIContext;
        std::weak_ptr<GameObject> CurrObject;
        NotificationManager *NotificationContext;
};

}

#endif // GAMECONTEXT_H
