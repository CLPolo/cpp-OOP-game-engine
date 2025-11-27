#include "Player.h"
#include "Bullet.h"
#include "Wall.h"
#include "Explosion.h"
#include "Enemy.h"
#include "Base.h"
#include <cstdlib>
#include <ctime>

using namespace CMPUT350;
// Base
Base::Base(Point2D loc)
: mBounds(loc, .5f * BASE_SIZE),
  mPos(loc),
  isAlive(true),
  mCore(Circle(loc, BASE_SIZE * 0.25f)),

  // Vertical rectangles (Left & Right)
  mRectLeft(Point2D(loc.x + BASE_SIZE * rectLeftPosX,
                    loc.y + BASE_SIZE * rectLeftPosY),
            BASE_SIZE * rectLRWidth,
            BASE_SIZE * rectLRHeight),

  mRectRight(Point2D(loc.x + BASE_SIZE * rectRightPosX,
                     loc.y + BASE_SIZE * rectRightPosY),
             BASE_SIZE * rectLRWidth,
             BASE_SIZE * rectLRHeight),

  // Horizontal rectangles (Top & Bottom)
  mRectTop(Point2D(loc.x + BASE_SIZE * rectTopPosX,
                   loc.y + BASE_SIZE * rectTopPosY),
           BASE_SIZE * rectTBWidth,
           BASE_SIZE * rectTBHeight),

  mRectBottom(Point2D(loc.x + BASE_SIZE * rectBottomPosX,
                      loc.y + BASE_SIZE * rectBottomPosY),
              BASE_SIZE * rectTBWidth,
              BASE_SIZE * rectTBHeight)
{

    // updating the frame (collsion) as the Rects shrink
    float minX = std::min({mRectLeft.topLeft.x, 
                       mRectRight.topLeft.x, 
                       mRectTop.topLeft.x, 
                       mRectBottom.topLeft.x});
    float maxX = std::max({mRectLeft.topLeft.x + mRectLeft.width,
                       mRectRight.topLeft.x + mRectRight.width,
                       mRectTop.topLeft.x + mRectTop.width,
                       mRectBottom.topLeft.x + mRectBottom.width});
    float minY = std::min({mRectLeft.topLeft.y, 
                       mRectRight.topLeft.y, 
                       mRectTop.topLeft.y, 
                       mRectBottom.topLeft.y});
    float maxY = std::max({mRectLeft.topLeft.y + mRectLeft.height,
                       mRectRight.topLeft.y + mRectRight.height,
                       mRectTop.topLeft.y + mRectTop.height,
                       mRectBottom.topLeft.y + mRectBottom.height});

    Point2D frameTopLeft(minX, minY);
    float frameWidth = maxX - minX;
    float frameHeight = maxY - minY;

    mFrame = Rect(frameTopLeft, frameWidth, frameHeight);
    mBounds = mFrame;
}
// GameObject
void Base::Update(GameContext *context)
{
    if (isAlive)
    {
        //Every 2 seconds, spawn an enemy
        double timeElapsed = mClock.getElapsedTime().asSeconds();
        if (timeElapsed >= 2.0)
        {
            mClock.restart();
            float angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
            float distance = BASE_SIZE * 0.5f;
            Point2D spawnLocation = Point2D(mPos.x + distance * cos(angle),
                                            mPos.y + distance * sin(angle));
            if (mContext && mContext->EngineContext)
            {
                auto enemy = std::make_shared<Enemy>(this, spawnLocation);
                
                // initial direction
                float angles[] = {0.f, M_PI / 4.f,M_PI / 2.f,3.f * M_PI / 4.f,M_PI,-3.f * M_PI / 4.f,-M_PI / 2.f,-M_PI / 4.f};
                int angleIndex = rand() % 8;
                float selectedAngle = angles[angleIndex];

                int direction = (rand() % 2 == 0) ? 1 : -1;
                selectedAngle *= direction;

                Point2D chosenDirection = Point2D(cos(selectedAngle), sin(selectedAngle));

                const float CHECK_DISTANCE = 200.0f; // Adjust as needed
                const float CHECK_SIZE = 50.0f; 

                Point2D rectCenter = mPos + chosenDirection * (CHECK_DISTANCE * 0.5f);
                Point2D rectTopLeft(rectCenter.x - CHECK_SIZE * 0.5f, rectCenter.y - CHECK_SIZE * 0.5f);
                CMPUT350::Rect checkRect(rectTopLeft, CHECK_DISTANCE, CHECK_SIZE);

                // rotate the rectangle depending on whether the direction is horizontal or vertical
                if (fabs(chosenDirection.x) > fabs(chosenDirection.y)) {
                    // Horizontal: make width long, height short
                    checkRect.width = CHECK_DISTANCE;
                    checkRect.height = CHECK_SIZE;
                } else {
                    // vertical: make height long, width short
                    checkRect.width = CHECK_SIZE;
                    checkRect.height = CHECK_DISTANCE;
                }
                CMPUT350::Shape checkShape(checkRect);

                bool wallFound = false;

                for (auto it = mContext->EngineContext->cbegin(); it != mContext->EngineContext->cend(); ++it) {
                if (std::dynamic_pointer_cast<Wall>(*it) != nullptr) {
                    auto wall = std::dynamic_pointer_cast<Wall>(*it);
                    const auto& shapes = wall->GetShapes();
                    for (const auto& wallShape : shapes) {
                        if (CMPUT350::CheckCollision(checkShape, wallShape)) {
                            wallFound = true;
                            break;
                        }
                        if (wallFound) break;
                    }
                }

                if (!wallFound) {
                    enemy->SetMoveDir(chosenDirection);
                    enemy->Rotate(selectedAngle);
                    mContext->EngineContext->AddGameObject(enemy);
                }
            
        }
    }

        // If health is less than 4, regen health every 2 seconds and update size
        if (baseHealth < 5 && timeElapsed >= 2.0)
        {
            baseHealth += 1;
            rectThickness += (initialThickness/4.0) ; // grow the base
            rectLRWidth = rectThickness;  
            rectTBHeight = rectThickness;
            offset *= rectLRWidth;

            // update the rectangles
            mRectLeft = Rect(Point2D(mPos.x + BASE_SIZE * rectLeftPosX,
                                     mPos.y + BASE_SIZE * rectLeftPosY),
                             BASE_SIZE * rectLRWidth,
                             BASE_SIZE * rectLRHeight);
            mRectRight = Rect(Point2D(mPos.x + BASE_SIZE * rectRightPosX,
                                      mPos.y + BASE_SIZE * rectRightPosY),
                              BASE_SIZE * rectLRWidth,
                              BASE_SIZE * rectLRHeight);
            mRectTop = Rect(Point2D(mPos.x + BASE_SIZE * rectTopPosX,
                                    mPos.y + BASE_SIZE * rectTopPosY),
                            BASE_SIZE * rectTBWidth,
                            BASE_SIZE * rectTBHeight);
            mRectBottom = Rect(Point2D(mPos.x + BASE_SIZE * rectBottomPosX,
                                       mPos.y + BASE_SIZE * rectBottomPosY),
                               BASE_SIZE * rectTBWidth,
                               BASE_SIZE * rectTBHeight);
            
            // updating the frame (collsion) as the Rects grows
            float minX = std::min({mRectLeft.topLeft.x, 
                           mRectRight.topLeft.x, 
                           mRectTop.topLeft.x, 
                           mRectBottom.topLeft.x});
            float maxX = std::max({mRectLeft.topLeft.x + mRectLeft.width,
                           mRectRight.topLeft.x + mRectRight.width,
                           mRectTop.topLeft.x + mRectTop.width,
                           mRectBottom.topLeft.x + mRectBottom.width});
            float minY = std::min({mRectLeft.topLeft.y, 
                           mRectRight.topLeft.y, 
                           mRectTop.topLeft.y, 
                           mRectBottom.topLeft.y});
            float maxY = std::max({mRectLeft.topLeft.y + mRectLeft.height,
                           mRectRight.topLeft.y + mRectRight.height,
                           mRectTop.topLeft.y + mRectTop.height,
                           mRectBottom.topLeft.y + mRectBottom.height});
            Point2D frameTopLeft(minX, minY);
            float frameWidth = maxX - minX;
            float frameHeight = maxY - minY;
            mFrame = Rect(frameTopLeft, frameWidth, frameHeight);
            mBounds = mFrame;
        }
         
        mContext = context;
        }
    }
}
void Base::LateUpdate(GameContext *context)
{
    
}
void Base::RenderBackground(GameContext *contextrender)
{
    if (isAlive)
    {
        // Render base core
        contextrender->ScreenContext->DrawCircle(mPos, BASE_SIZE*0.25f, Colors::red);
        // Render base outer rectangles
        contextrender->ScreenContext->DrawRect(mRectLeft, Colors::white);
        contextrender->ScreenContext->DrawRect(mRectRight, Colors::white);
        contextrender->ScreenContext->DrawRect(mRectTop, Colors::white);
        contextrender->ScreenContext->DrawRect(mRectBottom, Colors::white);
    }
}
void Base::RenderForeground(GameContext *contextrender)
{ 

}
bool Base::HandleKeyEvent(GameContext *context, char key)
{
    return false;
}
bool Base::IsAlive() { return isAlive; }
void Base::Kill() { isAlive = false;}
// CollisionObject
void Base::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
{
    // If bullet or player, reduce health and the size of the base
    auto bullet = std::dynamic_pointer_cast<Bullet>(obj);
    if (isAlive && bullet != nullptr && dynamic_cast<Enemy*>(bullet->mParent) == nullptr || std::dynamic_pointer_cast<Player>(obj))
    {
        

        rectThickness -= (initialThickness/4.0) ; // shrink the base
        rectLRWidth = rectThickness;  
        rectTBHeight = rectThickness;
        offset /= rectLRWidth; // adjust the offset accordingly

        baseHealth -= 1;


        // update the rectangles
        mRectLeft = Rect(Point2D(mPos.x + BASE_SIZE * rectLeftPosX,
                                 mPos.y + BASE_SIZE * rectLeftPosY),
                         BASE_SIZE * rectLRWidth,
                         BASE_SIZE * rectLRHeight);
        mRectRight = Rect(Point2D(mPos.x + BASE_SIZE * rectRightPosX,
                                  mPos.y + BASE_SIZE * rectRightPosY),
                          BASE_SIZE * rectLRWidth,
                          BASE_SIZE * rectLRHeight);
        mRectTop = Rect(Point2D(mPos.x + BASE_SIZE * rectTopPosX,
                                mPos.y + BASE_SIZE * rectTopPosY),
                        BASE_SIZE * rectTBWidth,
                        BASE_SIZE * rectTBHeight);
        mRectBottom = Rect(Point2D(mPos.x + BASE_SIZE * rectBottomPosX,
                                   mPos.y + BASE_SIZE * rectBottomPosY),
                           BASE_SIZE * rectTBWidth,
                           BASE_SIZE * rectTBHeight);
        
        // updating the frame (collsion) as the Rects shrink
        float minX = std::min({mRectLeft.topLeft.x, 
                       mRectRight.topLeft.x, 
                       mRectTop.topLeft.x, 
                       mRectBottom.topLeft.x});
        float maxX = std::max({mRectLeft.topLeft.x + mRectLeft.width,
                       mRectRight.topLeft.x + mRectRight.width,
                       mRectTop.topLeft.x + mRectTop.width,
                       mRectBottom.topLeft.x + mRectBottom.width});
        float minY = std::min({mRectLeft.topLeft.y, 
                       mRectRight.topLeft.y, 
                       mRectTop.topLeft.y, 
                       mRectBottom.topLeft.y});
        float maxY = std::max({mRectLeft.topLeft.y + mRectLeft.height,
                       mRectRight.topLeft.y + mRectRight.height,
                       mRectTop.topLeft.y + mRectTop.height,
                       mRectBottom.topLeft.y + mRectBottom.height});

        Point2D frameTopLeft(minX, minY);
        float frameWidth = maxX - minX;
        float frameHeight = maxY - minY;

        mFrame = Rect(frameTopLeft, frameWidth, frameHeight);
        mBounds = mFrame;

        if (baseHealth == 1)
        {
            mBounds = Rect(mCore.center, mCore.radius);
        }


        if (baseHealth <= 0)
        {
            isAlive = false;
            if (mContext && mContext->EngineContext)
            {
                auto explosion = std::make_shared<Explosion>(mPos, 100.f);
                mContext->EngineContext->AddGameObject(explosion);
            }
        }
    }

}
const std::vector<Shape> &Base::GetShapes()
{
    // Update shapes based on current health
    mShapes.clear();
    if (baseHealth <= 1) {
        // Only core is vulnerable when health is low
        mShapes.push_back(Shape(mCore));
    } else {
        // Frame is present when health > 1
        mShapes.push_back(Shape(mFrame));
        // Also include the rectangles that make up the frame
        mShapes.push_back(Shape(mRectLeft));
        mShapes.push_back(Shape(mRectRight));
        mShapes.push_back(Shape(mRectTop));
        mShapes.push_back(Shape(mRectBottom));
    }
    return mShapes;
}

const Rect &Base::GetBounds()
{
    return mBounds;
}