#include "Explosion.h"
#include "Bullet.h"
#include "GameContext.h"
#include <iostream>

Explosion::Explosion(CMPUT350::Point2D location, int radius)
    : CollisionObject(),
      mLocation(location),
      mMaxRadius(radius),
      mCurrentRadius(radius),
      mBounds(CMPUT350::Point2D(location.x - radius, location.y - radius), radius * 2, radius * 2)
{
    // Initialize shapes with a circle
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Circle(location, radius)));
}

void Explosion::Update(CMPUT350::GameContext* context)
{
    float deltaTime = mClock.restart().asSeconds();  // Get actual frame time
    mCurrentRadius -= SHRINK_RATE * deltaTime;

    // Check if explosion is done AFTER shrinking
    if (mCurrentRadius <= 0) {
        Kill();
        return;
    }

    // updating the bounds to match current radius to keep it centered
    mBounds = CMPUT350::Rect(
        CMPUT350::Point2D(mLocation.x - mCurrentRadius, mLocation.y - mCurrentRadius),
        mCurrentRadius * 2,
        mCurrentRadius * 2
    );

    // Update shapes to match current radius
    mShapes.clear();
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Circle(mLocation, mCurrentRadius)));
}

void Explosion::RenderForeground(CMPUT350::GameContext* context)
{
    if (mCurrentRadius > 0) {
        context->ScreenContext->DrawCircle(mLocation, mCurrentRadius, CMPUT350::Colors::cyan);
    }
}

bool Explosion::HandleKeyEvent(CMPUT350::GameContext* context, char key)
{
    return false;
}

void Explosion::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{


}

const CMPUT350::Rect& Explosion::GetBounds()
{
    mBounds = CMPUT350::Rect(
        CMPUT350::Point2D(mLocation.x - mCurrentRadius, mLocation.y - mCurrentRadius),
        mCurrentRadius * 2,
        mCurrentRadius * 2
    );
    return mBounds;
}

const std::vector<CMPUT350::Shape>& Explosion::GetShapes()
{
    return mShapes;
}
