
#ifndef GAMEENGINE_H
#define GAMEENGINE_H

namespace CMPUT350
{
class GameContext;
}

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <vector>
#include "MathUtil.h"
#include "GameObject.h"
#include "CollisionObject.h"
#include "DrawContext.h"
#include "EngineView.h"
#include "GameContext.h"
#include "NotificationManager.h"
#include "SpatialGrid.h"
#include <algorithm>

namespace CMPUT350
{

class GameEngine : public EngineView
{
public:
	GameEngine(unsigned int width, unsigned int height, const std::string& name);
	~GameEngine();

	GameEngine(const GameEngine&) = delete; // Prevent copy-construction
   	GameEngine& operator=(const GameEngine&) = delete; // Prevent assignment
   	GameEngine(GameEngine&&) = delete; // Prevent move-construction
   	GameEngine& operator=(GameEngine&&) = delete; // Prevent move-assignment

	void AddGameObject(std::shared_ptr<GameObject> gameObject) override;
	void Run();

	// EngineView iterator interface
	const_iterator cbegin() const override { return mGameObjects.cbegin(); }
	const_iterator cend() const override { return mGameObjects.cend(); }
	const_iterator begin() const override { return mGameObjects.begin(); }
	const_iterator end() const override { return mGameObjects.end(); }

private:
	std::vector<std::shared_ptr<GameObject>> mGameObjects;
	std::vector<std::shared_ptr<GameObject>> mAddedGameObjects;
	std::vector<std::shared_ptr<GameObject>> mRemovedGameObjects;

	std::shared_ptr<sf::RenderWindow> mWindow;
	std::unique_ptr<DrawContext> mScreenContext;
	std::unique_ptr<DrawContext> mGUIContext;
	NotificationManager mNotificationManager;
	GameContext mContext;

	bool mBoundingBoxes = false;
	bool mStep = false;
	bool mPaused = false;

	// Spatial partitioning grid for optimized static object collision detection
	SpatialGrid mSpatialGrid;
};
}

#endif // GAMEENGINE_H
