#include "Bullet.h"
#include "Explosion.h"
#include "GameContext.h"

using namespace CMPUT350;

 void Bullet::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
{
    // Default - use center of bullet
    CollisionEnter(obj, mShapeL.p1);  // or wherever your current bullet position is
}


void Bullet::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
{
    // the below assumes Collision has occured and been flagged by engine
    if (obj.get() == mParent){ return; } // Don't collide with papa

    if (mShapeL.p1 != collisionPoint)
    {
        mShapeL.p1 = collisionPoint;
        mShapeC = collisionPoint;
    }

    // Set flag to create explosion in LateUpdate
    mShouldExplode = true;
    mExplosionPos = collisionPoint;
    // std::cout << "Bullet collision at " << collisionPoint
    //          << " | Bullet pos: " << mShapeL.p1 << " prev: " << mShapeL.p2
    //          << " | lifetime: " << mLifetime << "\n";
    Kill();
    return;
}

void Bullet::Update(GameContext *context)
{
    // move and update positions
    mShapeL.p2 = this->mShapeL.p1;
    mShapeL.p1 += this->mDir * BULLET_SPEED;
    mShapeC.center = mShapeL.p1;
    mPrevPos = mShapeL.p2;

    // update bounds
    mBounds = Rect(mShapeL);
    if (mBounds.height == 0) // travelling horizontally
    {
        mBounds.topLeft.y -= mShapeC.radius;
        mBounds.height = mShapeL.width;
    }
    if (mBounds.width == 0) // travelling vertically
    {
        mBounds.topLeft.x -= mShapeC.radius;
        mBounds.width = mShapeL.width;
    }

    // std::cout << "Bullet counds: " << mBounds << "\n";
    // std::cout << "Bullet position: " << mShapeC.center << "\n";
    // Update shapes to match new position
    mShapes.clear();
    mShapes.push_back(Shape(mShapeL));
    mShapes.push_back(Shape(mShapeC));

    // update bullet lifetime. check if expired
    mLifetime -= BULLET_TICK;
    if (mLifetime <= 0.f)
    {
        Kill();
    }

    //_DebugOut();
}

void Bullet::LateUpdate(GameContext *context)
{
    // Create explosion if collision occurred
    if (mShouldExplode && context && context->EngineContext)
    {
        auto explosion = std::make_shared<Explosion>(mExplosionPos, 10);
        context->EngineContext->AddGameObject(explosion);
        mShouldExplode = false;
    }
}
void Bullet::RenderForeground(GameContext *context)
{
    context->ScreenContext->DrawCircle(
        this->mShapeL.p1, 
        this->mShapeL.width/2.0f, 
        CMPUT350::Colors::white
    );
}
