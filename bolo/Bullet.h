#ifndef BULLET_H
#define BULLET_H

#include "CollisionObject.h"
#include "GameContext.h"
#include "MathUtil.h"

using namespace CMPUT350;

const float BULLET_LIFETIME = 6.0f;
const float BULLET_SIZE = 3.0f;
const float BULLET_SPEED = 50.0f;
const float BULLET_TICK = 1.0f/30.f;

class Bullet : public CollisionObject
{
public:
    Bullet(GameObject *mParent, Point2D location, Point2D heading, float radius=BULLET_SIZE)
    : mParent(mParent), mPrevPos(location), mLifetime(BULLET_LIFETIME), mDir(heading)
    , isAlive(true), mBounds(location, radius)
    , mShapeL(Line(location, location, radius*2.0f)), mShapeC(Circle(location, radius))
    {
        // ensure mDir is unit vector
        mDir.Normalize();
        // Initialize shapes vector
        mShapes.push_back(Shape(mShapeL));
        mShapes.push_back(Shape(mShapeC));
    }
    ~Bullet()
    {
        mParent = nullptr;
    }

    // CollisionObject interface
    bool IsStatic() const override { return false; }
    const Rect &GetBounds() override { return mBounds; }
    const std::vector<Shape> &GetShapes() override { return mShapes; }

    bool IsAlive() { return isAlive; }
    void Kill() { isAlive = false; }
    void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override;
    void CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint) override;
    void RenderForeground(GameContext *context) override;
    void Update(GameContext *context) override;
    void LateUpdate(GameContext *context) override;
    
    GameObject* mParent;
    
private:

// void _DebugOut()
// {
//     std::cout << "Player state:"
//         << "\n\tPosition:\t"   << mShapeL.center
//         << "\n\tDirection:\t"  << mDir
//         << "\n\tisAlive:\t"    << isAlive
//     << std::endl;
// }

    Line mShapeL;
    Circle mShapeC;
    Rect mBounds;
    std::vector<Shape> mShapes;
    Point2D mDir;
    float mLifetime;
    bool isAlive;
    Point2D mPrevPos;

    bool mShouldExplode = false;
    Point2D mExplosionPos;
};
#endif // BULLET_H