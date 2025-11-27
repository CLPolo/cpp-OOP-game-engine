#include "Enemy.h"
#include "Player.h"
#include "Bullet.h"
#include "Wall.h"
#include "Explosion.h"
#include "Base.h"
#include <cstdlib>
#include <ctime>


Enemy::Enemy(GameObject *mParent,Point2D loc)
: mParent(mParent), mBounds(loc, .5f*ENEMY_SIZE), mFacingDir(0.f, -1.f)
, mPos(loc), mSpeed(0.f), isAlive(true), mMoveDir(0.f, -1.f), mBody(
    Point2D(loc.x - 0.5f * ENEMY_SIZE, loc.y),  // left
    Point2D(loc.x + 0.5f * ENEMY_SIZE, loc.y),  // right
    LINE_WIDTH
),
  mLine1(
      Point2D(loc.x + 0.43f * ENEMY_SIZE, loc.y - 0.3f * ENEMY_SIZE),
      Point2D(loc.x + 0.43f * ENEMY_SIZE, loc.y + 0.3f * ENEMY_SIZE),
      0.05f
  ),
  mLine2(
      Point2D(loc.x, loc.y- 0.8f * ENEMY_SIZE),
      Point2D(loc.x, loc.y),
      0.05f
  ),
  mLine3(
      Point2D(loc.x - 0.4f * ENEMY_SIZE, loc.y - 0.3f * ENEMY_SIZE),
      Point2D(loc.x - 0.4f * ENEMY_SIZE, loc.y + 0.3f * ENEMY_SIZE),
      0.05f
  )
{
    

}

void Enemy::Move(Point2D dir){
    mBody += dir;
    mLine1 += dir;
    mLine2 += dir;
    mLine3 += dir;
    mPos += dir;
    UpdateBounds();
}

void Enemy::Rotate(float angle){
    float currentSpeed = mMoveDir.length();
    mFacingDir.Rotate(ORIGIN, angle);
    mMoveDir = mFacingDir * currentSpeed;
    mBody.Rotate(mPos, angle);
    mLine1.Rotate(mPos, angle);
    mLine2.Rotate(mPos, angle);
    mLine3.Rotate(mPos, angle);
    UpdateBounds();
}

void Enemy::UpdateBounds(){
    mBounds = Rect(mPos, ENEMY_SIZE*.5f);
    mBounds |= mBody;
    mBounds |= mLine1;
    mBounds |= mLine2;
    mBounds |= mLine3;
}

void Enemy::SetMoveDir(Point2D dir){
    mMoveDir = dir;
}

bool Enemy::IsOutsideBase()
{
    // if (!mParent)
    // {
    //     std::cout << "mParent is null!" << std::endl;
    //     return true;
    // }else{
    //     std::cout << "mParent is not null!" << std::endl;
    // }

    // Cast mParent to Base pointer
    auto base = dynamic_cast<Base*>(mParent);
    if (!base) return true; // false check, no more enemy spawning without a base :(
    
    const Rect& baseBounds = base->GetBounds();
    
    // Check if enemy position is outside the base rectangle
    bool outsideLeft = mPos.x < baseBounds.topLeft.x;
    bool outsideRight = mPos.x > baseBounds.topLeft.x + baseBounds.width;
    bool outsideTop = mPos.y < baseBounds.topLeft.y;
    bool outsideBottom = mPos.y > baseBounds.topLeft.y + baseBounds.height;
    
    return outsideLeft || outsideRight || outsideTop || outsideBottom;
}

// GameObject
void Enemy::Update(GameContext *context)
{
    mContext = context;

    if (isAlive)
    { 
        // Start in a random direction
        // if (mSpeed == 0.f)
        // {
        //     mSpeed = 2.f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(3.f)));
        //     mMoveDir = mFacingDir * mSpeed;
        // }
        Move(Point2D(mMoveDir.x, mMoveDir.y));
    }
    else
    {   // blow up
        if (mContext && mContext->EngineContext)
        {
            auto explosion = std::make_shared<Explosion>(mPos, 300.f);
            mContext->EngineContext->AddGameObject(explosion);
        }
    }
    
    
    // Enemies have a 1/400 chance of changing their heading one step to the left or right each frame.
     if (IsOutsideBase())
    {
        int changeDirChance = rand() % 400;
        if (changeDirChance == 0)
        {
            float angles[] = {M_PI/4.f, M_PI/2.f, 3.f*M_PI/4.f};
            int angleIndex = rand() % 3;
            float selectedAngle = angles[angleIndex];
            int direction = (rand() % 2 == 0) ? 1 : -1;
            Rotate(direction * selectedAngle);
        }

        //  They have a 1/250 chance of firing a shot each frame.
        int fireChance = rand() % 250;
        if (fireChance == 0)
        {
            Point2D heading_bullet = mFacingDir*BULLET_SPEED;
            Point2D heading_enemy = mFacingDir*(mSpeed*5.f);
            auto bullet = std::make_shared<Bullet>
            (
                this, 
                mPos+(mFacingDir*ENEMY_SIZE*.5f),  
                heading_bullet + heading_enemy
            );
            context->EngineContext->AddGameObject(bullet);
        }
    }
    else{

    }




}
void Enemy::LateUpdate(GameContext *context)
{

}
void Enemy::RenderBackground(GameContext *contextrender)
{
    contextrender->ScreenContext->DrawLine(mBody.p1, mBody.p2, LINE_WIDTH, C_BODY);

    contextrender->ScreenContext->DrawLine(mLine1.p1, mLine1.p2, 5.0f, C_LINE);
    contextrender->ScreenContext->DrawLine(mLine2.p1, mLine2.p2, 7.0, C_LINE);
    contextrender->ScreenContext->DrawLine(mLine3.p1, mLine3.p2, 5.0f, C_LINE);

}
void Enemy::RenderForeground(GameContext *contextrender)
{ 

}
bool Enemy::HandleKeyEvent(GameContext *context, char key)
{
    return false; 

}
bool Enemy::IsAlive() { return isAlive; }
void Enemy::Kill() { isAlive = false;}



// CollisionObject
void Enemy::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
{
    // If collides with a wall, undo movement and turn randomly
    if (std::dynamic_pointer_cast<Wall>(obj) != nullptr)
    {
        // Undo last movement
        Move(Point2D(-mMoveDir.x, -mMoveDir.y));
        
        // Rotate randomly
        float angles[] = {M_PI/4.f, M_PI/2.f, 3.f*M_PI/4.f};
        int angleIndex = rand() % 3;
        float selectedAngle = angles[angleIndex];
        int direction = (rand() % 2 == 0) ? 1 : -1;
        Rotate(direction * selectedAngle);
    }

    if (std::dynamic_pointer_cast<Enemy>(obj) != nullptr)
{
    // Undo last movement
    Move(Point2D(-mMoveDir.x, -mMoveDir.y));
    
    // Turn randomly
    float angles[] = {M_PI/4.f, M_PI/2.f, 3.f*M_PI/4.f};
    int angleIndex = rand() % 3;
    float selectedAngle = angles[angleIndex];
    int direction = (rand() % 2 == 0) ? 1 : -1;
    Rotate(direction * selectedAngle);
}
    
    // If collides with a bullet, die and explode (if the bullet is from an enemy, ignore it)
    auto bullet = std::dynamic_pointer_cast<Bullet>(obj);
    if (bullet != nullptr && dynamic_cast<Enemy*>(bullet->mParent) == nullptr)
    {
        Kill();

        // Notify that enemy was killed
        if (mContext && mContext->NotificationContext)
        {
            mContext->NotificationContext->Notify("enemy_killed");
        }
    }
}
const std::vector<Shape>& Enemy::GetShapes() {
    // Update shapes with current positions
    mShapes.clear();
    // Enemy is represented as a circle for collision
    Circle enemyCircle(mPos, ENEMY_SIZE * 0.5f);
    mShapes.push_back(Shape(enemyCircle));
    return mShapes;
}
const Rect& Enemy::GetBounds() { return mBounds; }