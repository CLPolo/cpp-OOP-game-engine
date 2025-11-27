#include "SpatialGrid.h"
#include <algorithm>
/*
    This is adapted from https://gameprogrammingpatterns.com/spatial-partition.html
    and influenced by "Use of Residency Masks and Object Space Partitioning
                       To Eliminate Ray-Object Intersection Calculations"
    from Graphic Gems III, Ch 6.3
*/

namespace CMPUT350
{

SpatialGrid::SpatialGrid(int cellSize, int gridWidth, int gridHeight)
    : mCellSize(cellSize)
    , mGridWidth(gridWidth)
    , mGridHeight(gridHeight)
{}

int SpatialGrid::CellHash(int cellX, int cellY) const
{
    // Simple hash: cellX + cellY * gridWidth
    // Ensures unique hash for each cell in the grid
    return cellX + cellY * mGridWidth;
}

void SpatialGrid::Insert(std::shared_ptr<CollisionObject> obj)
{
    if (!obj) return;

    // Get object bounds
    const Rect& bounds = obj->GetBounds();

    // Calculate which grid cells this object occupies
    int minCellX = static_cast<int>(bounds.topLeft.x / mCellSize);
    int minCellY = static_cast<int>(bounds.topLeft.y / mCellSize);
    int maxCellX = static_cast<int>((bounds.topLeft.x + bounds.width) / mCellSize);
    int maxCellY = static_cast<int>((bounds.topLeft.y + bounds.height) / mCellSize);

    // Clamp to grid boundaries
    minCellX = std::max(0, std::min(minCellX, mGridWidth - 1));
    minCellY = std::max(0, std::min(minCellY, mGridHeight - 1));
    maxCellX = std::max(0, std::min(maxCellX, mGridWidth - 1));
    maxCellY = std::max(0, std::min(maxCellY, mGridHeight - 1));

    // Insert object into all cells it overlaps
    for (int y = minCellY; y <= maxCellY; ++y)
    {
        for (int x = minCellX; x <= maxCellX; ++x)
        {
            int hash = CellHash(x, y);
            mSpatialGrid[hash].push_back(obj);
        }
    }
}

std::vector<std::shared_ptr<CollisionObject>> SpatialGrid::QueryNearby(const Rect& bounds) const
{
    std::vector<std::shared_ptr<CollisionObject>> nearby;

    // Calculate center cell of the query bounds
    int centerCellX = static_cast<int>((bounds.topLeft.x + bounds.width / 2.0f) / mCellSize);
    int centerCellY = static_cast<int>((bounds.topLeft.y + bounds.height / 2.0f) / mCellSize);

    // Check 3x3 grid around the center cell
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int cellX = centerCellX + dx;
            int cellY = centerCellY + dy;

            // Skip out-of-bounds cells
            if (cellX < 0 || cellX >= mGridWidth || cellY < 0 || cellY >= mGridHeight)
                continue;

            int hash = CellHash(cellX, cellY);
            auto it = mSpatialGrid.find(hash);
            if (it != mSpatialGrid.end())
            {
                // Add all objects from this cell
                nearby.insert(nearby.end(), it->second.begin(), it->second.end());
            }
        }
    }

    return nearby;
}

void SpatialGrid::Clear()
{
    mSpatialGrid.clear();
}

} // namespace CMPUT350
