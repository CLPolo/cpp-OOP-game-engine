#include "Bolo.h"
#include "Player.h"
#include "Base.h"
#include "Wall.h"
#include <algorithm>
#include <random>
#include <iostream>

using namespace CMPUT350;

// ============================================================================
// CONSTRUCTOR & INITIALIZATION
// ============================================================================

Bolo::Bolo(float initialWindowWidth, float initialWindowHeight,
           int gridWidth, int gridHeight,
           int cellSize, int defaultDensity)
    : mGridWidth(gridWidth)
    , mGridHeight(gridHeight)
    , mCellSize(cellSize)
    , mDensity(defaultDensity)
    , mLevel(1)
    , mScore(0)
    , mBasesRemaining(0)
    , mPlayerDied(false)
    , mPlayerDeathPos(0, 0)
    , mCleanupFramesWaited(0)
    , mState(GameState::PreGame)
    , DEFAULT_GAME_SIZE(initialWindowHeight)  // Use initial height as reference
    , mScaleFactor(1.0f)
{
    // Initialize maze grid
    mMaze.resize(mGridHeight, std::vector<Cell>(mGridWidth));
    mOccupied.resize(mGridHeight, std::vector<bool>(mGridWidth, false));

    // Initialize layout
    mLayout.windowWidth = initialWindowWidth;
    mLayout.windowHeight = initialWindowHeight;
    CalculateLayout();

    // Start state timer
    mStateStartTime = std::chrono::steady_clock::now();

    std::cout << "Bolo initialized: " << mGridWidth << "x" << mGridHeight
              << " grid, cell size: " << mCellSize << "px, window: "
              << initialWindowWidth << "x" << initialWindowHeight << "\n";
}

// ============================================================================
// STATE MACHINE UPDATE
// ============================================================================

void Bolo::Update(GameContext* context)
{
    if (!context || !context->ScreenContext || !context->GUIContext)
        return;

    // Check window size
    CalculateWindowSize(context);

    switch (mState)
    {
        case GameState::PreGame:
            UpdatePreGame(context);
            break;
        case GameState::SetupLevel:
            UpdateSetupLevel(context);
            break;
        case GameState::Gameplay:
            UpdateGameplay(context);
            break;
        case GameState::EndLevel:
            UpdateEndLevel(context);
            break;
        case GameState::Cleanup:
            UpdateCleanup(context);
            break;
    }
}

// ============================================================================
// STATE HANDLERS
// ============================================================================

void Bolo::UpdateSetupLevel(GameContext* context)
{
    std::cout << "Setting up level " << mLevel << " with density " << mDensity << "\n";

    // Generate maze
    GenerateMaze(context);

    // Reset occupied grid
    for (auto& row : mOccupied)
        std::fill(row.begin(), row.end(), false);

    // Spawn bases
    SpawnBases(context);

    // Spawn player
    SpawnPlayer(context);

    // Register for notifications
    if (context && context->NotificationContext)
    {
        context->NotificationContext->Register(weak_from_this(), "enemy_killed");
        context->NotificationContext->Register(weak_from_this(), "base_destroyed");
    }

    // Transition to gameplay
    mState = GameState::Gameplay;
    mStateStartTime = std::chrono::steady_clock::now();
    std::cout << "Level setup complete. Starting gameplay.\n";
}

void Bolo::UpdatePreGame(GameContext* context)
{
    // Center camera on world center so PreGame message is visible
    if (context && context->ScreenContext)
    {
        float worldCenterX = (mGridWidth * mCellSize) / 2.0f;
        float worldCenterY = (mGridHeight * mCellSize) / 2.0f;
        context->ScreenContext->SetContextCenter(Point2D(worldCenterX, worldCenterY));
    }
    // Waiting for user to select density (1-5)
    // Transition happens in HandleKeyEvent
}

void Bolo::UpdateGameplay(GameContext* context)
{
    // Update screen center to follow player
    auto player = mPlayer.lock();
    if (player && context && context->ScreenContext)
    {   
        Point2D playerPos = player->GetPosition();
        context->ScreenContext->SetContextCenter(playerPos);
    }

    // Check if level is complete
    CheckLevelComplete(context);
}

void Bolo::UpdateEndLevel(GameContext* context)
{
    // Set camera position based on whether player died or completed level
    if (context && context->ScreenContext)
    {
        if (mPlayerDied)
        {
            // Center camera on death position
            context->ScreenContext->SetContextCenter(mPlayerDeathPos);
        }
        else
        {
            // Player won - try to center on player, fall back to world center
            auto player = mPlayer.lock();
            if (player)
            {
                context->ScreenContext->SetContextCenter(player->GetPosition());
            }
            else
            {
                // Fallback to world center
                float worldCenterX = (mGridWidth * mCellSize) / 2.0f;
                float worldCenterY = (mGridHeight * mCellSize) / 2.0f;
                context->ScreenContext->SetContextCenter(Point2D(worldCenterX, worldCenterY));
            }
        }
    }

    // Display message for END_LEVEL_DURATION seconds
    float elapsed = GetElapsedTime();

    if (elapsed >= END_LEVEL_DURATION)
    {
        std::cout << "End level timer complete. Starting cleanup.\n";
        mState = GameState::Cleanup;
        mCleanupFramesWaited = 0;
        mStateStartTime = std::chrono::steady_clock::now();
    }
}

void Bolo::UpdateCleanup(GameContext* context)
{
    // Kill all objects every frame (catches newly spawned objects too)
    int objectCount = 0;
    if (context && context->EngineContext)
    {
        for (auto it = context->EngineContext->begin(); it != context->EngineContext->end(); ++it)
        {
            auto obj = *it;
            if (obj.get() != this)
            {
                obj->Kill();
            }
            objectCount++;
        }
    }

    mCleanupFramesWaited++;

    // Wait until all objects are removed (only Bolo remains) AND minimum frames have passed
    if (mCleanupFramesWaited >= MIN_CLEANUP_FRAMES && objectCount <= 1)
    {
        // Only clear tracking data once at the end
        if (mCleanupFramesWaited == MIN_CLEANUP_FRAMES)
        {
            // Clear tracking
            mBases.clear();
            mPlayer.reset();

            // // Unregister from notifications
            // if (context && context->NotificationContext)
            // {
            //     context->NotificationContext->Unregister(weak_from_this(), "enemy_killed");
            //     context->NotificationContext->Unregister(weak_from_this(), "base_destroyed");
            // }
        }

        bool playerDied = mPlayerDied;  // Save before reset

        if (playerDied)
        {
            // Player died - return to PreGame state
            std::cout << "Cleanup complete (" << mCleanupFramesWaited << " frames, " << objectCount << " objects). Returning to PreGame.\n";

            // Reset death tracking
            mPlayerDied = false;
            mPlayerDeathPos = Point2D(0, 0);

            // Reset score and level
            mScore = 0;
            mLevel = 1;

            mState = GameState::PreGame;
            mStateStartTime = std::chrono::steady_clock::now();

            // Center camera on world center for PreGame
            if (context && context->ScreenContext)
            {
                float worldCenterX = (mGridWidth * mCellSize) / 2.0f;
                float worldCenterY = (mGridHeight * mCellSize) / 2.0f;
                context->ScreenContext->SetContextCenter(Point2D(worldCenterX, worldCenterY));
            }
        }
        else
        {
            // Player won - move to next level
            mLevel++;
            std::cout << "Cleanup complete (" << mCleanupFramesWaited << " frames, " << objectCount << " objects). Moving to level " << mLevel << "\n";

            // Reset death tracking
            mPlayerDied = false;
            mPlayerDeathPos = Point2D(0, 0);

            mState = GameState::SetupLevel;
            mStateStartTime = std::chrono::steady_clock::now();
        }
    }
}

// ============================================================================
// MAZE GENERATION (DFS ALGORITHM)
// ============================================================================

void Bolo::GenerateMaze(GameContext *context)
{
    // Initialize all walls present
    for (int r = 0; r < mGridHeight; r++)
    {
        for (int c = 0; c < mGridWidth; c++)
        {
            mMaze[r][c].wallTop = true;
            mMaze[r][c].wallRight = true;
            mMaze[r][c].wallBottom = true;
            mMaze[r][c].wallLeft = true;
        }
    }

    // Create visited grid for DFS
    std::vector<std::vector<bool>> visited(mGridHeight, std::vector<bool>(mGridWidth, false));

    // Start DFS from random cell
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rowDist(0, mGridHeight - 1);
    std::uniform_int_distribution<> colDist(0, mGridWidth - 1);

    int startRow = rowDist(gen);
    int startCol = colDist(gen);

    DFSMaze(startRow, startCol, visited);

    std::cout << "Maze generated using DFS\n";

    RemoveWallsForDensity();
    AddWallsToEngine(context);
}

void Bolo::DFSMaze(int row, int col, std::vector<std::vector<bool>>& visited)
{
    // Iterative DFS using explicit stack to avoid stack overflow
    // Stack holds (current cell, parent cell) to track which wall to remove
    std::random_device rd;
    std::mt19937 gen(rd());

    struct StackItem {
        int row, col;
        int parentRow, parentCol;
    };

    std::vector<StackItem> stack;
    stack.push_back({row, col, -1, -1});  // Start cell has no parent

    while (!stack.empty())
    {
        auto current = stack.back();
        stack.pop_back();

        // Skip if already visited
        if (visited[current.row][current.col])
            continue;

        // Mark as visited
        visited[current.row][current.col] = true;

        // Remove wall between current and parent (if parent exists)
        if (current.parentRow != -1)
        {
            int dr = current.row - current.parentRow;
            int dc = current.col - current.parentCol;

            if (dr == -1)  // Moved up
            {
                mMaze[current.parentRow][current.parentCol].wallTop = false;
                mMaze[current.row][current.col].wallBottom = false;
            }
            else if (dr == 1)  // Moved down
            {
                mMaze[current.parentRow][current.parentCol].wallBottom = false;
                mMaze[current.row][current.col].wallTop = false;
            }
            else if (dc == -1)  // Moved left
            {
                mMaze[current.parentRow][current.parentCol].wallLeft = false;
                mMaze[current.row][current.col].wallRight = false;
            }
            else if (dc == 1)  // Moved right
            {
                mMaze[current.parentRow][current.parentCol].wallRight = false;
                mMaze[current.row][current.col].wallLeft = false;
            }
        }

        // Define directions: up, right, down, left
        std::vector<std::pair<int, int>> directions = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

        // Randomize direction order for this cell
        std::shuffle(directions.begin(), directions.end(), gen);

        // Add all unvisited neighbors to stack with current cell as parent
        for (auto [dr, dc] : directions)
        {
            int newRow = current.row + dr;
            int newCol = current.col + dc;

            // Check bounds
            if (newRow < 0 || newRow >= mGridHeight || newCol < 0 || newCol >= mGridWidth)
                continue;

            // Check if already visited
            if (visited[newRow][newCol])
                continue;

            // Push neighbor with current cell as parent
            stack.push_back({newRow, newCol, current.row, current.col});
        }
    }
}

void Bolo::RemoveWallsForDensity()
{
    // Count current walls (after DFS maze creation)
    int mazeWalls = CountWalls();

    // Calculate target walls based on density formula: (5 + 2*density) / 18
    // Apply formula to the maze wall count (not total possible walls)
    float keepRatio = (5.0f + 2.0f * mDensity) / 18.0f;
    int targetWalls = static_cast<int>(mazeWalls * keepRatio);

    std::cout << "DFS maze walls: " << mazeWalls << ", Target walls: " << targetWalls
              << " (density " << mDensity << ", keep ratio: " << keepRatio << ")\n";

    // If already at or below target, done
    if (mazeWalls <= targetWalls)
        return;

    int currentWalls = mazeWalls;

    // Remove random interior walls until we reach target
    std::random_device rd;
    std::mt19937 gen(rd());

    int wallsToRemove = currentWalls - targetWalls;
    int removed = 0;

    while (removed < wallsToRemove)
    {
        // Pick random interior cell (not on outer border)
        std::uniform_int_distribution<> rowDist(1, mGridHeight - 2);
        std::uniform_int_distribution<> colDist(1, mGridWidth - 2);
        int r = rowDist(gen);
        int c = colDist(gen);

        // Try to remove a random wall
        std::uniform_int_distribution<> wallDist(0, 3);
        int wall = wallDist(gen);

        bool wallRemoved = false;
        switch (wall)
        {
            case 0: // Top
                if (mMaze[r][c].wallTop && r > 0)
                {
                    mMaze[r][c].wallTop = false;
                    mMaze[r-1][c].wallBottom = false;
                    wallRemoved = true;
                }
                break;
            case 1: // Right
                if (mMaze[r][c].wallRight && c < mGridWidth - 1)
                {
                    mMaze[r][c].wallRight = false;
                    mMaze[r][c+1].wallLeft = false;
                    wallRemoved = true;
                }
                break;
            case 2: // Bottom
                if (mMaze[r][c].wallBottom && r < mGridHeight - 1)
                {
                    mMaze[r][c].wallBottom = false;
                    mMaze[r+1][c].wallTop = false;
                    wallRemoved = true;
                }
                break;
            case 3: // Left
                if (mMaze[r][c].wallLeft && c > 0)
                {
                    mMaze[r][c].wallLeft = false;
                    mMaze[r][c-1].wallRight = false;
                    wallRemoved = true;
                }
                break;
        }

        if (wallRemoved)
            removed++;
    }

    std::cout << "Removed " << removed << " walls for density adjustment\n";
}

int Bolo::CountWalls()
{
    int count = 0;
    for (int r = 0; r < mGridHeight; r++)
    {
        for (int c = 0; c < mGridWidth; c++)
        {
            // Only count top and right walls to avoid double-counting
            if (mMaze[r][c].wallTop) count++;
            if (mMaze[r][c].wallRight) count++;
        }
    }
    return count;
}

void Bolo::AddWallsToEngine(GameContext* context)
{
    if (!context || !context->EngineContext)
        return;

    int wallsAdded = 0;
    int interiorWalls = 0;
    int borderWalls = 0;

    // Add horizontal walls (top edges of cells)
    for (int r = 0; r < mGridHeight; r++)
    {
        for (int c = 0; c < mGridWidth; c++)
        {
            if (mMaze[r][c].wallTop)
            {
                Point2D origin = GridToWorld(r, c);
                auto wall = std::make_shared<Wall>(origin, static_cast<float>(mCellSize), kHorizontal, 5.0f);
                context->EngineContext->AddGameObject(wall);
                wallsAdded++;
                if (r > 0 && r < mGridHeight - 1 && c > 0 && c < mGridWidth - 1)
                    interiorWalls++;
            }
        }
    }

    // Add vertical walls (left edges of cells)
    for (int r = 0; r < mGridHeight; r++)
    {
        for (int c = 0; c < mGridWidth; c++)
        {
            if (mMaze[r][c].wallLeft)
            {
                Point2D origin = GridToWorld(r, c);
                auto wall = std::make_shared<Wall>(origin, static_cast<float>(mCellSize), kVertical, 5.0f);
                context->EngineContext->AddGameObject(wall);
                wallsAdded++;
                if (r > 0 && r < mGridHeight - 1 && c > 0 && c < mGridWidth - 1)
                    interiorWalls++;
            }
        }
    }

    // Add right border (closing wall for rightmost column)
    for (int r = 0; r < mGridHeight; r++)
    {
        Point2D origin = GridToWorld(r, mGridWidth);
        auto wall = std::make_shared<Wall>(origin, static_cast<float>(mCellSize), kVertical, 5.0f);
        context->EngineContext->AddGameObject(wall);
        wallsAdded++;
        borderWalls++;
    }

    // Add bottom border (closing wall for bottom row)
    for (int c = 0; c < mGridWidth; c++)
    {
        Point2D origin = GridToWorld(mGridHeight, c);
        auto wall = std::make_shared<Wall>(origin, static_cast<float>(mCellSize), kHorizontal, 5.0f);
        context->EngineContext->AddGameObject(wall);
        wallsAdded++;
        borderWalls++;
    }

    std::cout << "Added " << wallsAdded << " wall objects to engine (interior: " << interiorWalls
              << ", border: " << borderWalls << ")\n";
}

// ============================================================================
// OBJECT SPAWNING
// ============================================================================

void Bolo::SpawnBases(GameContext* context)
{
    if (!context || !context->EngineContext)
        return;

    mBases.clear();
    mBasesRemaining = BASES_PER_LEVEL;

    for (int i = 0; i < BASES_PER_LEVEL; i++)
    {
        Point2D cellPos = GetRandomUnoccupiedCell();
        Point2D worldPos = GridToWorld(static_cast<int>(cellPos.y), static_cast<int>(cellPos.x));

        // Center in cell
        worldPos.x += mCellSize / 2.0f;
        worldPos.y += mCellSize / 2.0f;

        auto base = std::make_shared<Base>(worldPos);
        context->EngineContext->AddGameObject(base);
        mBases.push_back(base);
    }

    std::cout << "Spawned " << BASES_PER_LEVEL << " bases\n";
}

void Bolo::SpawnPlayer(GameContext* context)
{
    if (!context || !context->EngineContext)
        return;

    Point2D cellPos = GetRandomUnoccupiedCell();
    Point2D worldPos = GridToWorld(static_cast<int>(cellPos.y), static_cast<int>(cellPos.x));

    // Center in cell
    worldPos.x += mCellSize / 2.0f;
    worldPos.y += mCellSize / 2.0f;

    auto player = std::make_shared<Player>(worldPos);
    context->EngineContext->AddGameObject(player);
    mPlayer = player;

    std::cout << "Spawned player at grid (" << cellPos.x << ", " << cellPos.y
              << ") world (" << worldPos.x << ", " << worldPos.y << ")\n";
}

Point2D Bolo::GetRandomUnoccupiedCell()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rowDist(0, mGridHeight - 1);
    std::uniform_int_distribution<> colDist(0, mGridWidth - 1);

    while (true)
    {
        int r = rowDist(gen);
        int c = colDist(gen);

        if (!mOccupied[r][c])
        {
            mOccupied[r][c] = true;
            return Point2D(static_cast<float>(c), static_cast<float>(r));  // Return as (col, row)
        }
    }
}

Point2D Bolo::GridToWorld(int row, int col)
{
    return Point2D(static_cast<float>(col * mCellSize), static_cast<float>(row * mCellSize));
}

void Bolo::WorldToGrid(Point2D worldPos, int& row, int& col)
{
    col = static_cast<int>(worldPos.x / mCellSize);
    row = static_cast<int>(worldPos.y / mCellSize);
}

// ============================================================================
// GAME LOGIC
// ============================================================================

void Bolo::CheckLevelComplete(GameContext* context)
{
    int remaining = CountRemainingBases(context);
    mBasesRemaining = remaining;

    if (remaining == 0)
    {
        // Level complete - player won
        std::cout << "Level " << mLevel << " complete! Final score: " << mScore << "\n";
        mPlayerDied = false;
        mState = GameState::EndLevel;
        mStateStartTime = std::chrono::steady_clock::now();
    }
    else
    {
        // Check if player died
        auto player = mPlayer.lock();
        if (!player || !player->IsAlive())
        {
            std::cout << "You died. Final score: " << mScore << "\n";

            // Store death position (use last known position or world center)
            if (player)
            {
                mPlayerDeathPos = player->GetPosition();
            }
            else
            {
                // Player completely destroyed - use world center
                mPlayerDeathPos = Point2D((mGridWidth * mCellSize) / 2.0f,
                                          (mGridHeight * mCellSize) / 2.0f);
            }

            mPlayerDied = true;
            mState = GameState::EndLevel;
            mStateStartTime = std::chrono::steady_clock::now();
        }
    }
}

int Bolo::CountRemainingBases(GameContext* context)
{
    // Clean up expired weak_ptrs and count alive bases
    int count = 0;
    for (auto it = mBases.begin(); it != mBases.end(); )
    {
        auto base = it->lock();
        if (!base)
        {
            it = mBases.erase(it);
        }
        else
        {
            if (base->IsAlive())
                count++;
            ++it;
        }
    }
    return count;
}

// void Bolo::CleanupLevel(GameContext* context)
// {
//     // Note: This function is now called from UpdateCleanup
//     // Object killing is handled in UpdateCleanup every frame
//     // This just provides a centralized place for future cleanup logic if needed
// }

// ============================================================================
// INPUT HANDLING
// ============================================================================

bool Bolo::HandleKeyEvent(GameContext* context, char key)
{
    if (mState == GameState::PreGame)
    {
        // Check for density selection (1-5)
        if (key >= '1' && key <= '5')
        {
            mDensity = key - '0';
            std::cout << "Density " << mDensity << " selected. Starting game...\n";
            mState = GameState::SetupLevel;
            mStateStartTime = std::chrono::steady_clock::now();
            return true;
        }
    }

    return false;
}

// ============================================================================
// NOTIFICATIONS
// ============================================================================

void Bolo::ReceiveNotification(const std::string& message)
{
    if (message == "enemy_killed")
    {
        mScore += 1;
        std::cout << "Enemy killed! Score: " << mScore << "\n";
    }
    else if (message == "base_destroyed")
    {
        mScore += 100;
        std::cout << "Base destroyed! Score: " << mScore << ", Bases remaining: " << mBasesRemaining << "\n";
    }
}

// ============================================================================
// TIMING HELPER
// ============================================================================

float Bolo::GetElapsedTime() const
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStateStartTime);
    return duration.count() / 1000.0f;  // Convert to seconds
}

// ============================================================================
// LAYOUT CALCULATION
// ============================================================================

void Bolo::CalculateWindowSize(GameContext* context)
{
        // Check for window resize
    Point2D windowSize = context->GUIContext->GetWindowSize();
    if (windowSize.x != mLayout.windowWidth || windowSize.y != mLayout.windowHeight)
    {   
        mLayout.windowWidth = windowSize.x;
        mLayout.windowHeight = windowSize.y;
        CalculateLayout();
        std::cout << "Window resized to " << windowSize.x << "x" << windowSize.y << "\n";
    }

    // Configure viewports for this frame with centering
    float totalContentWidth = mLayout.gameplayWidth + mLayout.guiWidth;
    float totalContentHeight = mLayout.gameplayWidth;  // Height same as gameplay (square)

    // Calculate letterboxing offsets (normalized 0-1)
    float horizontalLetterbox = (mLayout.windowWidth - totalContentWidth) / 2.0f;
    float verticalLetterbox = (mLayout.windowHeight - totalContentHeight) / 2.0f;
    float leftOffset = horizontalLetterbox / mLayout.windowWidth;
    float topOffset = verticalLetterbox / mLayout.windowHeight;

    // Viewport dimensions (normalized)
    float gameplayViewportWidth = mLayout.gameplayWidth / mLayout.windowWidth;
    float gameplayViewportHeight = mLayout.gameplayWidth / mLayout.windowHeight;
    float guiViewportWidth = mLayout.guiWidth / mLayout.windowWidth;

    // ScreenContext: Centered square viewport (gameplay area)
    context->ScreenContext->SetViewport(leftOffset, topOffset, gameplayViewportWidth, gameplayViewportHeight);
    context->ScreenContext->SetViewSize(mLayout.gameplayWidth, mLayout.gameplayWidth);

    // GUIContext: Right of gameplay, same vertical position
    float guiViewportStart = leftOffset + gameplayViewportWidth;
    context->GUIContext->SetViewport(guiViewportStart, topOffset, guiViewportWidth, gameplayViewportHeight);
    context->GUIContext->SetViewSize(mLayout.guiWidth, mLayout.gameplayWidth);  // Same height as gameplay
    context->GUIContext->SetContextCenter(Point2D(mLayout.guiWidth / 2.0f, mLayout.gameplayWidth / 2.0f));
}

void Bolo::CalculateLayout()
{
    // Layout constraints:
    // - Gameplay: square (3x3 ratio)
    // - GUI: 1/3 of gameplay width, same height (1x3 ratio)
    // - Total content: 4:3 aspect ratio

    // Determine which dimension limits us
    float windowAspect = mLayout.windowWidth / mLayout.windowHeight;
    const float CONTENT_ASPECT = 4.0f / 3.0f;  // Total content is 4:3

    float gameplaySize;  // Both width and height (square)

    if (windowAspect >= CONTENT_ASPECT)
    {
        // Window is wider than 4:3 - height is limiting dimension
        // Center horizontally with letterboxing on sides
        gameplaySize = mLayout.windowHeight;
    }
    else
    {
        // Window is taller than 4:3 - width is limiting dimension
        // Center vertically with letterboxing on top/bottom
        gameplaySize = mLayout.windowWidth * 0.75f;  // 3/4 of width for gameplay
    }

    mLayout.gameplayWidth = gameplaySize;
    mLayout.guiWidth = gameplaySize / 3.0f;  // GUI is 1/3 the size of gameplay
    mLayout.guiStartX = mLayout.gameplayWidth;

    // Calculate GUI element positions WITHIN the GUI viewport (0 to guiWidth x 0 to gameplayWidth)
    // GUIContext uses coordinates relative to its viewport, not absolute window coords
    float guiCenterX = mLayout.guiWidth * 0.5f;
    float leftMargin = mLayout.guiWidth * 0.1f;
    float guiHeight = mLayout.gameplayWidth;  // GUI height matches gameplay height

    // Update scale factor based on current gameplay size
    mScaleFactor = mLayout.gameplayWidth / DEFAULT_GAME_SIZE;

    // Title: Top center of GUI
    mLayout.titlePos = Point2D(guiCenterX - 50 * mScaleFactor, guiHeight * 0.05f);

    // Radar: Below title, left-aligned with margin
    mLayout.radarPos = Point2D(leftMargin, guiHeight * 0.15f);
    mLayout.radarSize = std::min(mLayout.guiWidth * 0.6f, guiHeight * 0.2f);

    // Direction finder: Below radar
    mLayout.directionFinderPos = Point2D(leftMargin,
                                          mLayout.radarPos.y + mLayout.radarSize + 20 * mScaleFactor);
    mLayout.directionFinderSize = mLayout.radarSize * 0.5f;

    // Score and base count: Lower portion
    mLayout.scorePos = Point2D(leftMargin, guiHeight * 0.7f);
    mLayout.baseCountPos = Point2D(leftMargin, guiHeight * 0.75f);

    // Messages: Center of gameplay area
    mLayout.preGameMessagePos = Point2D(mLayout.gameplayWidth * 0.2f,
                                         mLayout.gameplayWidth * 0.5f);
    mLayout.endLevelMessagePos = Point2D(mLayout.gameplayWidth * 0.25f,
                                          mLayout.gameplayWidth * 0.5f);

    // Calculate scaled text sizes
    mLayout.titleTextSize = static_cast<int>(48 * mScaleFactor);
    mLayout.guiTextSize = static_cast<int>(24 * mScaleFactor);
    mLayout.guiSmallTextSize = static_cast<int>(18 * mScaleFactor);
    mLayout.messageTextSize = static_cast<int>(48 * mScaleFactor);
    mLayout.messageSmallTextSize = static_cast<int>(24 * mScaleFactor);
}


// ============================================================================
// GUI RENDERING (USES CALCULATED LAYOUT)
// ============================================================================

void Bolo::RenderForeground(GameContext* context)
{
    if (!context || !context->GUIContext || !context->ScreenContext)
        return;

    // Apply GUIContext view for GUI rendering
    context->GUIContext->ApplyView();

    // Always render GUI
    RenderGUI(context);

    // State-specific rendering in ScreenContext (messages appear in gameplay area)
    context->ScreenContext->ApplyView();  // Switch back to gameplay view for messages

    if (mState == GameState::PreGame)
    {
        // Messages in PreGame should be centered on screen in world coordinates
        // For PreGame, we don't have a player yet, so center on world center
        float worldCenterX = (mGridWidth * mCellSize) / 2.0f;
        float worldCenterY = (mGridHeight * mCellSize) / 2.0f;

        context->ScreenContext->DrawText("Press 1-5 to select maze density", mLayout.guiTextSize,
                                      Point2D(worldCenterX - 200, worldCenterY),
                                      Colors::white);

        std::string densityInfo = "Current: " + std::to_string(mDensity);
        context->ScreenContext->DrawText(densityInfo, mLayout.guiSmallTextSize,
                                      Point2D(worldCenterX - 50, worldCenterY + 40),
                                      Colors::cyan);
    }
    else if (mState == GameState::EndLevel)
    {
        if (mPlayerDied)
        {
            // Player died - show death message
            context->ScreenContext->DrawText("YOU DIED!", mLayout.messageTextSize,
                                          Point2D(mPlayerDeathPos.x - 150, mPlayerDeathPos.y - 50),
                                          Colors::red);
        }
        else
        {
            // Player won - show victory message
            auto player = mPlayer.lock();
            Point2D messagePos;

            if (player)
            {
                messagePos = player->GetPosition();
            }
            else
            {
                // Fallback to world center
                messagePos = Point2D((mGridWidth * mCellSize) / 2.0f,
                                    (mGridHeight * mCellSize) / 2.0f);
            }

            context->ScreenContext->DrawText("LEVEL COMPLETE!", mLayout.messageTextSize,
                                          Point2D(messagePos.x - 150, messagePos.y - 50),
                                          Colors::green);
            std::string nextLevel = "Next level in " +
                                   std::to_string(static_cast<int>(END_LEVEL_DURATION - GetElapsedTime() + 1)) + "s";
            context->ScreenContext->DrawText(nextLevel, mLayout.messageSmallTextSize,
                                          Point2D(messagePos.x - 100, messagePos.y + 10),
                                          Colors::white);
        }
    }
}

void Bolo::RenderGUI(GameContext* context)
{
    // Draw GUI background (dark gray rectangle covering entire GUI area)
    Rect guiBackground(Point2D(0, 0), mLayout.guiWidth, mLayout.gameplayWidth);
    context->GUIContext->DrawRect(guiBackground, RGBColor(30, 30, 30));

    RenderTitle(context);
    RenderScore(context);
    RenderBaseCount(context);
    RenderRadar(context);
    RenderDirectionFinder(context);
}

void Bolo::RenderTitle(GameContext* context)
{
    context->GUIContext->DrawText("BOLO", mLayout.titleTextSize, mLayout.titlePos, Colors::white);

    std::string levelText = "Level " + std::to_string(mLevel);
    context->GUIContext->DrawText(levelText, mLayout.guiSmallTextSize,
                                  Point2D(mLayout.titlePos.x + 20 * mScaleFactor,
                                          mLayout.titlePos.y + 60 * mScaleFactor),
                                  Colors::cyan);
}

void Bolo::RenderScore(GameContext* context)
{
    std::string scoreText = "Score: " + std::to_string(mScore);
    context->GUIContext->DrawText(scoreText, mLayout.guiTextSize, mLayout.scorePos, Colors::white);
}

void Bolo::RenderBaseCount(GameContext* context)
{
    std::string baseText = "Bases: " + std::to_string(mBasesRemaining) + "/" +
                           std::to_string(BASES_PER_LEVEL);
    context->GUIContext->DrawText(baseText, mLayout.guiTextSize, mLayout.baseCountPos, Colors::white);
}

void Bolo::RenderRadar(GameContext* context)
{
    // Draw radar box
    Rect radarBox(mLayout.radarPos, mLayout.radarSize, mLayout.radarSize);
    context->GUIContext->DrawRect(radarBox, Colors::gray);

    // Draw player position dot on radar
    auto player = mPlayer.lock();
    if (player && mState == GameState::Gameplay)
    {
        float worldWidth = mGridWidth * mCellSize;
        float worldHeight = mGridHeight * mCellSize;
        Point2D playerPos = player->GetPosition();

        // Scale player world position to radar coordinates
        Point2D radarDot(
            mLayout.radarPos.x + (playerPos.x / worldWidth) * mLayout.radarSize,
            mLayout.radarPos.y + (playerPos.y / worldHeight) * mLayout.radarSize
        );

        // Draw player as green dot on radar
        float dotRadius = 3 * mScaleFactor;
        context->GUIContext->DrawCircle(radarDot, dotRadius, Colors::green);
    }
}

void Bolo::RenderDirectionFinder(GameContext* context)
{
    // Draw direction finder label
    context->GUIContext->DrawText("Base Finder", mLayout.guiSmallTextSize, mLayout.directionFinderPos, Colors::white);

    // Get player position
    auto player = mPlayer.lock();
    if (!player || mState != GameState::Gameplay)
    {
        // No player or not in gameplay - show gray grid
        float cellSize = mLayout.directionFinderSize / 2.0f;
        Point2D gridStart(mLayout.directionFinderPos.x, mLayout.directionFinderPos.y + 25 * mScaleFactor);
        float cellBorder = 2 * mScaleFactor;

        for (int row = 0; row < 2; row++)
        {
            for (int col = 0; col < 2; col++)
            {
                Rect cell(Point2D(gridStart.x + col * cellSize, gridStart.y + row * cellSize),
                          cellSize - cellBorder, cellSize - cellBorder);
                context->GUIContext->DrawRect(cell, Colors::gray);
            }
        }
        return;
    }

    Point2D playerPos = player->GetPosition();

    // Determine which quadrants have bases (inclusive boundaries)
    bool topLeft = false;
    bool topRight = false;
    bool bottomLeft = false;
    bool bottomRight = false;

    for (const auto& weakBase : mBases)
    {
        auto base = weakBase.lock();
        if (!base || !base->IsAlive())
            continue;

        Point2D basePos = base->GetPosition();

        // Check each quadrant (inclusive comparisons)
        if (basePos.y <= playerPos.y && basePos.x <= playerPos.x)
            topLeft = true;
        if (basePos.y <= playerPos.y && basePos.x >= playerPos.x)
            topRight = true;
        if (basePos.y >= playerPos.y && basePos.x <= playerPos.x)
            bottomLeft = true;
        if (basePos.y >= playerPos.y && basePos.x >= playerPos.x)
            bottomRight = true;
    }

    // Draw 2x2 grid with appropriate colors
    float cellSize = mLayout.directionFinderSize / 2.0f;
    Point2D gridStart(mLayout.directionFinderPos.x, mLayout.directionFinderPos.y + 25 * mScaleFactor);
    float cellBorder = 2 * mScaleFactor;

    // Top-left quadrant
    Rect tlCell(gridStart, cellSize - cellBorder, cellSize - cellBorder);
    context->GUIContext->DrawRect(tlCell, topLeft ? Colors::white : Colors::gray);

    // Top-right quadrant
    Rect trCell(Point2D(gridStart.x + cellSize, gridStart.y),
                cellSize - cellBorder, cellSize - cellBorder);
    context->GUIContext->DrawRect(trCell, topRight ? Colors::white : Colors::gray);

    // Bottom-left quadrant
    Rect blCell(Point2D(gridStart.x, gridStart.y + cellSize),
                cellSize - cellBorder, cellSize - cellBorder);
    context->GUIContext->DrawRect(blCell, bottomLeft ? Colors::white : Colors::gray);

    // Bottom-right quadrant
    Rect brCell(Point2D(gridStart.x + cellSize, gridStart.y + cellSize),
                cellSize - cellBorder, cellSize - cellBorder);
    context->GUIContext->DrawRect(brCell, bottomRight ? Colors::white : Colors::gray);
}
