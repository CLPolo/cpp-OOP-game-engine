#include <cassert>
#include "Player.h"
#include "Bullet.h"
#include "Wall.h"
#include "Explosion.h"
#include "Enemy.h"

using namespace CMPUT350;

// Player
Player::Player(Point2D loc)
: mBounds(loc, .5f*PLAYER_SIZE), mFacingDir(0.f, -1.f) // Player starts facing up
, mPos(loc), mSpeed(0.f), isAlive(true), mMoveDir(0.f, -1.f)
, mTurretDir(0.f, -1.f), mTurret(loc, Point2D(loc.x, loc.y-PLAYER_SIZE), LINE_WIDTH)
, mFramesSinceLastShot(MIN_SHOT_DELAY)  // Start ready to shoot
, mTreadL(
    Point2D(loc.x + .5f*PLAYER_SIZE - .5f*LINE_WIDTH, loc.y - .5f*PLAYER_SIZE), 
    PLAYER_SIZE, 
    LINE_WIDTH, 
    M_PI_2
)
, mTreadR(
    Point2D(loc.x - .5f*PLAYER_SIZE + .5f*LINE_WIDTH, loc.y - .5f*PLAYER_SIZE), 
    PLAYER_SIZE, 
    LINE_WIDTH, 
    M_PI_2
)
, mCockpit(
    Point2D(loc.x, loc.y - .25f*PLAYER_SIZE),
    Point2D(loc.x, loc.y + .25f*PLAYER_SIZE),
    LINE_WIDTH
)
{
    // Initialize shapes vector with player's lines
    mShapes.push_back(Shape(mTurret));
    mShapes.push_back(Shape(mTreadL));
    mShapes.push_back(Shape(mTreadR));
    mShapes.push_back(Shape(mCockpit));
}

void Player::Rotate(float angle)
{
    // Rotates the Player's body
    mFacingDir.Rotate(ORIGIN, angle);
    mMoveDir = mFacingDir*sgn(mSpeed);
    mTreadL.Rotate(mPos, angle);
    mTreadR.Rotate(mPos, angle);
    mCockpit.Rotate(mPos, angle);

    // Update shapes after rotation
    mShapes.clear();
    mShapes.push_back(Shape(mTurret));
    mShapes.push_back(Shape(mTreadL));
    mShapes.push_back(Shape(mTreadR));
    mShapes.push_back(Shape(mCockpit));
}

void Player::Move(Point2D dir)
{
    mTreadL += dir;
    mTreadR += dir;
    mCockpit += dir;
    mTurret += dir;
    mPos += dir;
    _UpdateBounds();
    _UpdateShapes();
}

void Player::_UpdateBounds()
{
    // The bounds is simply the union of the Player's shapes
    mBounds = Rect(mPos, PLAYER_SIZE*.5f);
    mBounds |= mTreadL;
    mBounds |= mTreadR;
    mBounds |= mCockpit;
    mBounds |= mTurret;
}

void Player::_UpdateShapes()
{
    // Update shapes after movement
    mShapes.clear();
    mShapes.push_back(Shape(mTurret));
    mShapes.push_back(Shape(mTreadL));
    mShapes.push_back(Shape(mTreadR));
    mShapes.push_back(Shape(mCockpit));
}

void Player::_DebugOut()
{
    std::cout << "Player state:"
        << "\n\tPosition:\t"   << mPos 
        << "\n\tDirection:\t"  << mFacingDir
        << "\n\tMove Dir:\t"  << mMoveDir
        << "\n\tSpeed:\t"      << mSpeed
        << "\n\tSpeed Sign:\t" << sgn(mSpeed)
        << "\n\tTurret Dir:\t" << mTurretDir 
        << "\n\tTreadL P1:\t"  << mTreadL.p1
        << "\n\tTreadL P2:\t"  << mTreadL.p2
        << "\n\tisAlive:\t"    << isAlive
    << std::endl;
}

void Player::Update(GameContext *context)
{
    if (isAlive)
    {
        Move(mFacingDir*(mSpeed*SPEED_INTERVAL));

        // Increment shot delay counter (max out at MIN_SHOT_DELAY to prevent overflow)
        if (mFramesSinceLastShot < MIN_SHOT_DELAY)
        {
            mFramesSinceLastShot++;
        }

#ifdef DEBUG
        _DebugOut();
#endif
    }
}

void Player::LateUpdate(GameContext *context)
{
    if (!isAlive)
    {   // blow up
        if (context && context->EngineContext)
        {
            auto explosion = std::make_shared<Explosion>(mPos, PLAYER_SIZE);
            context->EngineContext->AddGameObject(explosion);
        }
    }
}
void Player::RenderBackground(GameContext *contextrender)
{   
    if (isAlive)
    {
        // Render treads and cockpit
        contextrender->ScreenContext->DrawLine( mTreadL.p1,  mTreadL.p2, LINE_WIDTH, C_TREAD);
        contextrender->ScreenContext->DrawLine( mTreadR.p1,  mTreadR.p2, LINE_WIDTH, C_TREAD);
        contextrender->ScreenContext->DrawLine(mCockpit.p1, mCockpit.p2, LINE_WIDTH, C_COCKPIT);
    }
}
void Player::RenderForeground(GameContext *contextrender)
{   
    if (isAlive)
    {
        // Render turret
        contextrender->ScreenContext->DrawLine(mTurret.p1, mTurret.p2, LINE_WIDTH, C_TURRET);
    }
}
bool Player::HandleKeyEvent(GameContext *context, char key)
{
    switch (key)
    {
        case 'w':
            if (mSpeed < MAX_SPEED) mSpeed += 1;
            return true;
        case 's':
            mSpeed = 0.f;
            return true;
        case 'x':
            if (mSpeed > -MAX_SPEED) mSpeed -= 1;
            return true;
        case 'a':
            Rotate(-ROTATION_STEP);
            mTurret.Rotate(mPos, -ROTATION_STEP);
            mTurretDir.Rotate(ORIGIN, -ROTATION_STEP);
            _UpdateBounds();
            return true;
        case 'd':
            Rotate(ROTATION_STEP);
            mTurret.Rotate(mPos, ROTATION_STEP);
            mTurretDir.Rotate(ORIGIN, ROTATION_STEP);
            _UpdateBounds();
            return true;
        case '1':
            mTurret.Rotate(mPos, -ROTATION_STEP);
            mTurretDir.Rotate(ORIGIN, -ROTATION_STEP);
            _UpdateBounds();
            return true;
        case '2':
            mTurret.Rotate(mPos, ROTATION_STEP);
            mTurretDir.Rotate(ORIGIN, ROTATION_STEP);
            _UpdateBounds();
            return true;
        case ' ':
        {
            // Only shoot if minimum delay has passed
            if (mFramesSinceLastShot >= MIN_SHOT_DELAY)
            {
                Point2D heading_bullet = mTurretDir*BULLET_SPEED;
                Point2D heading_tank = mFacingDir*(mSpeed*SPEED_INTERVAL);
                auto bullet = std::make_shared<Bullet>
                (
                    this,
                    mTurret.p2,  // Spawn at actual end of turret barrel
                    heading_bullet + heading_tank
                );
                context->EngineContext->AddGameObject(bullet);

                // Reset shot delay counter
                mFramesSinceLastShot = 0;
            }
            return true;
        }
        default:
            return false;
    }
}
bool Player::IsAlive() { return isAlive; }
void Player::Kill() { isAlive = false;}

// CollisionObject
void Player::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
{
    // Check if the colliding object is a Bullet, Wall, or Enemy
    auto bullet = std::dynamic_pointer_cast<Bullet>(obj);
    auto wall = std::dynamic_pointer_cast<Wall>(obj);
    auto enemy = std::dynamic_pointer_cast<Enemy>(obj);

    if ((bullet && bullet->mParent != this) || wall || enemy)
    {
        Kill();
    }
}
const std::vector<Shape>& Player::GetShapes() { return mShapes; }
const Rect& Player::GetBounds() { return mBounds; }