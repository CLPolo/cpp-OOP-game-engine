#include "Wall.h"

using namespace CMPUT350;

Wall::Wall(Point2D origin, float length, tWallDirection dir, float width)
: origin(origin)
, length(length)
, width(width)
, shape(
    Line(origin,
        dir == kHorizontal // determine wall shape endpoint
            ? Point2D(origin.x + length, origin.y)
            : Point2D(origin.x, origin.y + length),
        width
    )
)
, mBounds(
    dir == kHorizontal
        ? Rect(Point2D(origin.x, origin.y - .5f*width), length, width)
        : Rect(Point2D(origin.x - 0.5f*width, origin.y), width, length)
)
{
    // Initialize shapes vector with the wall's line
    mShapes.push_back(Shape(shape));
}

void Wall::RenderForeground(GameContext *context)
{   
    context->ScreenContext->DrawRect(mBounds, Colors::gray);
}