#ifndef SPATIALGRID_H
#define SPATIALGRID_H

#include <vector>
#include <unordered_map>
#include <memory>
#include "CollisionObject.h"
#include "MathUtil.h"

namespace CMPUT350
{

/**
 * Spatial partitioning grid for efficient collision detection
 *
 * Divides the game world into a uniform grid of cells. Static objects
 * are inserted once, and dynamic objects query nearby cells to find
 * potential collision candidates.
 */
class SpatialGrid
{
public:
    /**
     * Construct a spatial grid
     * @param cellSize Size of each grid cell in pixels (should match world cell size)
     * @param gridWidth Number of cells horizontally
     * @param gridHeight Number of cells vertically
     */
    SpatialGrid(int cellSize, int gridWidth, int gridHeight);

    /**
     * Insert a static object into the grid
     * @param obj The collision object to insert (typically a wall or base)
     *
     * Objects are inserted into all cells their bounding box overlaps.
     * Should only be called for static objects that don't move.
     */
    void Insert(std::shared_ptr<CollisionObject> obj);

    /**
     * Query nearby objects within a bounding box
     * @param bounds The query rectangle (typically a dynamic object's bounds)
     * @return Vector of objects in nearby cells (may contain duplicates)
     *
     * Checks a 3x3 grid of cells around the query bounds center.
     * Returns all static objects that could potentially collide.
     */
    std::vector<std::shared_ptr<CollisionObject>> QueryNearby(const Rect& bounds) const;

    /**
     * Clear all objects from the grid
     *
     * Should be called when rebuilding the grid (e.g., level transitions)
     */
    void Clear();

    /**
     *Get statistics about the grid (for debugging)
     */
    int GetTotalCells() const { return mGridWidth * mGridHeight; }
    int GetOccupiedCells() const { return mSpatialGrid.size(); }

private:
    int mCellSize;
    int mGridWidth;
    int mGridHeight;

    // Hash map: cell hash -> list of collision objects in that cell
    std::unordered_map<int, std::vector<std::shared_ptr<CollisionObject>>> mSpatialGrid;

    /**
     * Compute hash for a grid cell
     * @param cellX Cell X coordinate
     * @param cellY Cell Y coordinate
     * @return Unique integer hash for the cell
     */
    int CellHash(int cellX, int cellY) const;
};

} // namespace CMPUT350

#endif // SPATIALGRID_H
