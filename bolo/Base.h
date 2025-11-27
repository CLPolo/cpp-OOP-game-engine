#ifndef BASE_H
#define BASE_H


#include "CollisionObject.h"

using namespace CMPUT350;
class Base : public CollisionObject{
public:
    Base(Point2D loc);

    // GameObject
    void Update(GameContext *context);
    void LateUpdate(GameContext *context);
    void RenderBackground(GameContext *contextrender);
    void RenderForeground(GameContext *contextrender);
    bool HandleKeyEvent(GameContext *context, char key);
    bool IsAlive();
    void Kill();

    // CollisionObject interface
    bool IsStatic() const override { return true; }
    void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override;
    const std::vector<Shape> &GetShapes() override;
    const Rect &GetBounds() override;
    Point2D GetPosition() const { return mPos; }
private:
    const float BASE_SIZE = 30.f;
    Rect mBounds;
    std::vector<Shape> mShapes;
    Point2D mPos;
    bool isAlive;
    GameContext* mContext = nullptr;
    Rect mFrame = Rect(mPos, BASE_SIZE*10, BASE_SIZE*10);
    Circle mCore = Circle(mPos, BASE_SIZE*0.25f);

    // Frame thickness (relative to BASE_SIZE)
    float rectThickness = 0.5f;  

    // Change the values with *** to make the Base bigger or smaller


    float rectLRWidth = rectThickness;  
    float rectLRHeight = 6.0f;  // more distance from core ***


    float rectTBWidth = 6.0f;  //more distance from core ***
    float rectTBHeight = rectThickness;


    float offset = 3.0f; // how far the box extends from center *** (Make it half of the hight/widht)

    float rectLeftPosX = -offset - rectThickness * 0.5f;
    float rectLeftPosY = -offset;

    float rectRightPosX = offset - rectThickness * 0.5f;
    float rectRightPosY = -offset;

    float rectTopPosX = -offset;
    float rectTopPosY = -offset - rectThickness * 0.5f;

    float rectBottomPosX = -offset;
    float rectBottomPosY = offset - rectThickness * 0.5f;

    int baseHealth = 5;
    sf::Clock mClock;

    float initialThickness = rectThickness;

    Rect mRectLeft, mRectRight, mRectTop,mRectBottom;

};



#endif