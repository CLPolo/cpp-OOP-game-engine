#ifndef BOLO_H
#define BOLO_H

#include "CollisionObject.h"
#include "Player.h"
#include "Base.h"
#include "Wall.h"
#include <vector>
#include <memory>
#include <chrono>

using namespace CMPUT350;

class Bolo : public GameObject, public std::enable_shared_from_this<Bolo>
{
    public:
    Bolo(float initialWindowWidth, float initialWindowHeight,
    int gridWidth = 58, int gridHeight = 58, int cellSize = 256, int defaultDensity = 3);

    // GameObject overrides
    void Update(GameContext* context) override;
    void RenderForeground(GameContext* context) override;
    bool HandleKeyEvent(GameContext* context, char key) override;
    void ReceiveNotification(const std::string& message) override;

private:
    // Game state machine
    enum class GameState {
        PreGame,      // Waiting for density selection
        SetupLevel,   // Creating maze, spawning objects
        Gameplay,     // Active gameplay
        EndLevel,     // Level complete message
        Cleanup       // Removing all objects
    };

    GameState mState;

    // State update handlers
    void UpdatePreGame(GameContext* context);
    void UpdateSetupLevel(GameContext* context);
    void UpdateGameplay(GameContext* context);
    void UpdateEndLevel(GameContext* context);
    void UpdateCleanup(GameContext* context);

    // Maze configuration
    int mGridWidth;         // 58
    int mGridHeight;        // 58
    int mCellSize;          // 256 pixels
    int mDensity;           // 1-5 (wall density)
    int mLevel;             // Current level number

    // Layout scaling
    const float DEFAULT_GAME_SIZE;  // Reference size for scaling (set from initial window height)
    float mScaleFactor;             // Current scale factor (gameplayWidth / DEFAULT_GAME_SIZE)

    // Maze data structure - cell connectivity
    struct Cell {
        bool wallTop;
        bool wallRight;
        bool wallBottom;
        bool wallLeft;
    };
    std::vector<std::vector<Cell>> mMaze;  // [row][col]

    // Maze generation
    void GenerateMaze(GameContext *context);
    void DFSMaze(int row, int col, std::vector<std::vector<bool>>& visited);
    void RemoveWallsForDensity();
    void AddWallsToEngine(GameContext* context);
    int CountWalls();

    // Object spawning and tracking
    void SpawnBases(GameContext* context);
    void SpawnPlayer(GameContext* context);
    Point2D GridToWorld(int row, int col);
    void WorldToGrid(Point2D worldPos, int& row, int& col);
    Point2D GetRandomUnoccupiedCell();

    std::vector<std::vector<bool>> mOccupied;  // Track occupied cells
    std::weak_ptr<Player> mPlayer;
    std::vector<std::weak_ptr<Base>> mBases;

    // Score & game stats
    int mScore;
    int mBasesRemaining;
    const int BASES_PER_LEVEL = 6;

    // Player death tracking
    bool mPlayerDied;           // True if player died (vs level complete)
    Point2D mPlayerDeathPos;    // Player position when they died

    // Layout management (resize-ready)
    struct LayoutParams {
        float windowWidth;
        float windowHeight;
        float gameplayWidth;    // Left section for gameplay
        float guiWidth;         // Right section for GUI
        float guiStartX;        // X coordinate where GUI begins

        // GUI element positions (calculated from window size)
        Point2D titlePos;
        Point2D radarPos;
        float radarSize;
        Point2D directionFinderPos;
        float directionFinderSize;
        Point2D scorePos;
        Point2D baseCountPos;
        Point2D preGameMessagePos;
        Point2D endLevelMessagePos;

        // Scaled text sizes (proportional to layout)
        int titleTextSize;
        int guiTextSize;
        int guiSmallTextSize;
        int messageTextSize;
        int messageSmallTextSize;
    };

    LayoutParams mLayout;

    void CalculateWindowSize(GameContext* context);

    // Calculate layout based on window size
    void CalculateLayout();

    // Called by engine when window resizes
    void UpdateLayout(float windowWidth, float windowHeight);

    // GUI rendering (uses mLayout for positions)
    void RenderGUI(GameContext* context);
    void RenderTitle(GameContext* context);
    void RenderRadar(GameContext* context);
    void RenderDirectionFinder(GameContext* context);
    void RenderScore(GameContext* context);
    void RenderBaseCount(GameContext* context);

    // Timing (using standard C++ chrono, no SFML dependency)
    std::chrono::steady_clock::time_point mStateStartTime;
    int mCleanupFramesWaited;
    const float END_LEVEL_DURATION = 4.0f;  // seconds
    const int MIN_CLEANUP_FRAMES = 5;       // frames to wait after level

    // Helper to get elapsed time since state start
    float GetElapsedTime() const;

    // Game logic
    void CheckLevelComplete(GameContext* context);
    void CleanupLevel(GameContext* context);
    int CountRemainingBases(GameContext* context);
};

#endif // BOLO_H
