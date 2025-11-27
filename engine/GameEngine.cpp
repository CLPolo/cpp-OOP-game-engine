#include "GameEngine.h"
#include <iostream>
#include <cstdlib> 

/// @brief 
namespace CMPUT350
{
#include "FontData.h"

// Constructor
GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string& name)
    : mSpatialGrid(256, 58, 58)  // 256px cells, 58x58 grid (matches Bolo maze)
{
    // Configure the SFML window
    mWindow = std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), name);
    mWindow->setFramerateLimit(30);
    //mWindow->setKeyRepeatEnabled(false);

    // Create and load font
    auto font = std::make_shared<sf::Font>();
    if (!font->openFromMemory(
        _System_Library_Fonts_Supplemental_Trattatello_ttf,
        sizeof(_System_Library_Fonts_Supplemental_Trattatello_ttf))
    ){
        std::cout << __FILE__ << ":" << __LINE__ << " font failed to load, exiting\n";
        exit(EXIT_FAILURE);
    }

    // Create DrawContexts with unique_ptr (ownership in GameEngine)
    mScreenContext = std::make_unique<DrawContext>(mWindow, font);
    mGUIContext = std::make_unique<DrawContext>(mWindow, font);

    // Initialize GameContext with raw pointers (non-owning, as per spec)
    mContext.EngineContext = this;
    mContext.ScreenContext = mScreenContext.get();
    mContext.GUIContext = mGUIContext.get();
    mContext.NotificationContext = &mNotificationManager;
    mContext.CurrObject = std::weak_ptr<GameObject>();  // Will be set per-object during iteration
}

// Destructor
GameEngine::~GameEngine()
{
    // unique_ptr will automatically clean up mScreenContext and mGUIContext
    if (mWindow->isOpen()) { mWindow->close(); }
}

// Adding a game object adds it to a temporary list of added objects that are added at the beginning of the next frame.
void GameEngine::AddGameObject(std::shared_ptr<GameObject> gameObject)
{
    mAddedGameObjects.push_back(gameObject);	
}

void GameEngine::Run()
{
    while(mWindow->isOpen())
    {
        // Skip frame processing if paused (unless stepping)
        if (mPaused && !mStep)
        {
            // Still process events when paused
            while (const std::optional<sf::Event> event = mWindow->pollEvent())
            {
                if (event->is<sf::Event::Closed>()) { mWindow->close(); }

                if (const auto* key = event->getIf<sf::Event::KeyPressed>())
                {
                    if (key->code == sf::Keyboard::Key::P) mPaused = !mPaused;
                    if (key->code == sf::Keyboard::Key::S) mStep = true;  // Step one frame
                }
            }
            continue;  // Skip game logic when paused
        }

        // If we're stepping, process one frame then pause again
        if (mStep)
        {
            mStep = false;
            // Will process this frame normally
        }

        //Remove any objects which are no longer alive.
        mGameObjects.erase(
            std::remove_if(
                mGameObjects.begin(),
                mGameObjects.end(),
                [](const std::shared_ptr<GameObject>& obj) { return !obj->IsAlive(); }),
            mGameObjects.end());
        
        // Remove expired listeners from notification manager
        mNotificationManager.CleanupExpired();

        //Add any objects that have been created in the last frame.
        if (!mAddedGameObjects.empty())
        {
            mGameObjects.insert(mGameObjects.end(), mAddedGameObjects.begin(), mAddedGameObjects.end());
            mAddedGameObjects.clear();
        }
        
        //Process any SFML events
        while (const std::optional<sf::Event> event = mWindow->pollEvent())
        {
            if (event->is<sf::Event::Closed>()) { mWindow->close(); }
                            
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::P) { mPaused = !mPaused; }
                if (key->code == sf::Keyboard::Key::B) { mBoundingBoxes = !mBoundingBoxes; }

            }       
            else if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
            {
                char key = static_cast<char>(textEntered->unicode);
                //std::cout << "Text entered: '" << key << "' (unicode: " << static_cast<uint32_t>(textEntered->unicode) << ")\n";
                for (auto& obj : mGameObjects) { obj->HandleKeyEvent(&mContext, key); }
            }
        }

        //Update all game objects
        for (auto& obj : mGameObjects) { obj->Update(&mContext); }

        //Process collisions (optimized for many static objects like walls)
        // Separate static and dynamic collision objects, and build spatial grid
        std::vector<std::shared_ptr<CollisionObject>> dynamicObjects;
        mSpatialGrid.Clear();  // Clear grid from previous frame

        for (auto& obj : mGameObjects)
        {
            auto collObj = std::dynamic_pointer_cast<CollisionObject>(obj);
            if (collObj && obj->IsAlive())
            {
                if (collObj->IsStatic())
                    mSpatialGrid.Insert(collObj);  // Add to spatial grid
                else
                    dynamicObjects.push_back(collObj);
            }
        }

        // Check dynamic-vs-dynamic collisions
        for (size_t a = 0; a < dynamicObjects.size(); ++a)
        {
            for (size_t b = a + 1; b < dynamicObjects.size(); ++b)
            {
                auto objA = dynamicObjects[a];
                auto objB = dynamicObjects[b];

                auto boundsA = objA->GetBounds();
                auto boundsB = objB->GetBounds();

                // First check bounding boxes for coarse intersection
                bool boundsIntersect = !(boundsA.topLeft.x + boundsA.width < boundsB.topLeft.x ||
                            boundsA.topLeft.x > boundsB.topLeft.x + boundsB.width ||
                            boundsA.topLeft.y + boundsA.height < boundsB.topLeft.y ||
                            boundsA.topLeft.y > boundsB.topLeft.y + boundsB.height);

                if (boundsIntersect)
                {
                    
                    const auto& shapesA = objA->GetShapes();
                    const auto& shapesB = objB->GetShapes();

                    bool collided = false;
                    Point2D collisionPoint(0, 0);

                    for (const auto& shapeA : shapesA)
                    {
                        for (const auto& shapeB : shapesB)
                        {
                            if (CheckCollision(shapeA, shapeB, &collisionPoint))
                            {   
                                //std::cout << "Returned from shape collision check. Collision at point " << collisionPoint << "\n";
                                collided = true;
                                break;
                            }
                        }
                        if (collided) break;
                    }

                    if (collided)
                    {
                        objA->CollisionEnter(objB, collisionPoint);
                        objB->CollisionEnter(objA, collisionPoint);
                    }
                }
            }
        }

        // Check dynamic-vs-static collisions using spatial grid
        for (auto& dynObj : dynamicObjects)
        {
            // Query spatial grid for nearby static objects
            auto nearbyStatic = mSpatialGrid.QueryNearby(dynObj->GetBounds());

            for (auto& statObj : nearbyStatic)
            {
                auto boundsA = dynObj->GetBounds();
                auto boundsB = statObj->GetBounds();

                // First check bounding boxes for coarse intersection
                bool boundsIntersect = !(boundsA.topLeft.x + boundsA.width < boundsB.topLeft.x ||
                            boundsA.topLeft.x > boundsB.topLeft.x + boundsB.width ||
                            boundsA.topLeft.y + boundsA.height < boundsB.topLeft.y ||
                            boundsA.topLeft.y > boundsB.topLeft.y + boundsB.height);

                if (boundsIntersect)
                {
                    // std::cout << "  Bounds overlap - checking shapes: dyn=" << boundsA << " stat=" << boundsB << "\n";
                    const auto& shapesA = dynObj->GetShapes();
                    const auto& shapesB = statObj->GetShapes();

                    bool collided = false;
                    Point2D collisionPoint(0, 0);

                    for (const auto& shapeA : shapesA)
                    {
                        for (const auto& shapeB : shapesB)
                        {
                            if (CheckCollision(shapeA, shapeB, &collisionPoint))
                            {
                                collided = true;
                                break;
                            }
                        }
                        if (collided) break;
                    }

                    if (collided)
                    {
                        //std::cout << "    COLLISION! Point=" << collisionPoint << "\n";
                        dynObj->CollisionEnter(statObj, collisionPoint);
                        statObj->CollisionEnter(dynObj, collisionPoint);
                    }
                }
            }
        }

        //Do late updates on all game objects
        for (auto& obj : mGameObjects) { obj->LateUpdate(&mContext); }

        //Render the background
        mWindow->clear(sf::Color::Black);

        // Apply ScreenContext view for gameplay rendering
        mContext.ScreenContext->ApplyView();

        //Render all game objects in ScreenContext
        for (auto& obj : mGameObjects)
        {
            obj->RenderBackground(&mContext);
        }
        for (auto& obj : mGameObjects)
        {
            obj->RenderForeground(&mContext);
            // draw bounding boxes if toggled
            if (mBoundingBoxes)
            {
                auto bb = std::dynamic_pointer_cast<CollisionObject>(obj);
                if (bb)
                {
                    Rect bounds = bb->GetBounds();
                    mContext.ScreenContext->DrawRect(bounds, Colors::magenta);
                }
            }
        }

        //Call display() on the window to show the frame.
        mWindow->display();
    }
}
}