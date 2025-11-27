#include "GameEngine.h"
#include "bolo/Bolo.h"

using namespace CMPUT350;

int main()
{
	// Window dimensions
	const unsigned int WINDOW_WIDTH = 1440;
	const unsigned int WINDOW_HEIGHT = 1080;

	// Create game engine
	GameEngine engine(WINDOW_WIDTH, WINDOW_HEIGHT, "Bolo");

	// Create Bolo game manager
	// Parameters: gridWidth, gridHeight, cellSize, defaultDensity, windowWidth, windowHeight
	auto bolo = std::make_shared<Bolo>( static_cast<float>(WINDOW_WIDTH),
	                                    static_cast<float>(WINDOW_HEIGHT)
	);

	engine.AddGameObject(bolo);

	// Run the game
	engine.Run();

	return 0;
}