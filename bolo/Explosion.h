#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "CollisionObject.h"
#include <SFML/System/Clock.hpp>

namespace CMPUT350
{

class Explosion : public CollisionObject
{
public:
    Explosion(Point2D location, int radius);

    void Update(GameContext* context) override;
    void RenderForeground(GameContext* context) override;
    bool HandleKeyEvent(GameContext* context, char key) override;

    // GameObject interface
    bool IsAlive() override { return mAlive; }
    void Kill() override { mAlive = false; }

    // CollisionObject interface
    bool IsStatic() const override { return true; }
    void CollisionEnter(const std::shared_ptr<CollisionObject>& obj) override;
    const Rect& GetBounds() override;
    const std::vector<Shape>& GetShapes() override;

private:

    Point2D mLocation;
    int mMaxRadius;
    int mCurrentRadius;
    Rect mBounds;
    std::vector<Shape> mShapes;
    bool mAlive = true;

    static constexpr float SHRINK_RATE = 100.0f;  // Explosions last ~1.5 seconds max
    sf::Clock mClock;

};
} // namespace CMPUT350
#endif // EXPLOSION_H