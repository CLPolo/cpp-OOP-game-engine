#include "CollisionObject.h"

namespace CMPUT350 {

void CollisionObject::CollisionEnter(const std::shared_ptr<CollisionObject> &obj)
{
    // Default implementation does nothing
    // Derived classes can override this if they don't need the collision point
}

void CollisionObject::CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint)
{
    // Default implementation just calls the version without collision point
    CollisionEnter(obj);
}

// ============================================================================
// Collision detection implementation, adapted from Project 1
// ============================================================================

bool CheckCollision(const Shape& s1, const Shape& s2, Point2D *collisionPoint)
{   
    // Nested switch on shape types
    switch (s1.t)
    {
        case ShapeType::kRect:
        {
            const Rect& r1 = s1.shape.rect;
            switch (s2.t)
            {   //=============================================
                // Rectangle x Rectangle
                //=============================================
                case ShapeType::kRect:
                {
                    const Rect& r2 = s2.shape.rect;
                    // Rect-Rect collision
                    bool separate = r1.topLeft.x + r1.width < r2.topLeft.x      // r2 to the right
                                    || r2.topLeft.x + r2.width < r1.topLeft.x   // r2 to the left
                                    || r1.topLeft.y + r1.height < r2.topLeft.y  // r2 below
                                    || r2.topLeft.y + r2.height < r1.topLeft.y; // r2 above

                    if (!separate && collisionPoint)
                    {
                        // Use intersection operator to get overlap rect
                        Rect intersection = r1;
                        intersection &= r2;
                        // Collision point is center of intersection
                        *collisionPoint = Point2D(
                            intersection.topLeft.x + intersection.width / 2.0f,
                            intersection.topLeft.y + intersection.height / 2.0f
                        );
                    }
                    return !separate;
                }
                //=============================================
                // Rectangle x Circle
                //=============================================
                case ShapeType::kCircle:
                {
                    const Circle& c = s2.shape.circle;
                    // Rect-Circle collision: find closest point on rect to circle center
                    float closestX = std::max(r1.topLeft.x, std::min(c.center.x, r1.topLeft.x + r1.width));
                    float closestY = std::max(r1.topLeft.y, std::min(c.center.y, r1.topLeft.y + r1.height));
                    Point2D closest(closestX, closestY);
                    bool collides = c.center.Distance(closest) <= c.radius;

                    if (collides && collisionPoint)
                    {
                        *collisionPoint = closest;
                    }
                    return collides;
                }
                //=============================================
                // Rectangle x Line
                //=============================================
                case ShapeType::kLine:
                {
                    const Line& l = s2.shape.line;
                    // Rect-Line collision: check if endpoints inside or line crosses edges
                    if (r1.IsInside(l.p1) || r1.IsInside(l.p2)) return true;

                    // Check intersection with each edge, accounting for line width
                    Line edges[4] = {
                        Line(
                            r1.topLeft, Point2D(r1.topLeft.x + r1.width, r1.topLeft.y)),  // top
                        Line(
                            r1.topLeft, Point2D(r1.topLeft.x, r1.topLeft.y + r1.height)), // left
                        Line(
                            Point2D(r1.topLeft.x, r1.topLeft.y + r1.height),              // bottom
                            Point2D(r1.topLeft.x + r1.width, r1.topLeft.y + r1.height)),
                        Line(
                            Point2D(r1.topLeft.x + r1.width, r1.topLeft.y),               // right
                            Point2D(r1.topLeft.x + r1.width, r1.topLeft.y + r1.height)
                        )
                    };

                    for (int i = 0; i < 4; i++)
                    {
                        Point2D tmp;
                        if (l.Crosses(edges[i], tmp))
                        {   
                            if (collisionPoint)
                                *collisionPoint = tmp;
                            return true;
                        }
                        // If line has width, also check distance
                        if (l.width > 0)
                        {
                            float d1 = l.ClosestPoint(edges[i].p1).Distance(edges[i].p1);
                            float d2 = l.ClosestPoint(edges[i].p2).Distance(edges[i].p2);
                            if (d1 <= l.width || d2 <= l.width)
                                return true;
                        }
                    }
                    return false;
                }
            }
            break;
        }

        case ShapeType::kCircle:
        {
            const Circle& c1 = s1.shape.circle;
            switch (s2.t)
            {   //=============================================
                // Circle x Circle
                //=============================================
                case ShapeType::kCircle:
                {
                    const Circle& c2 = s2.shape.circle;
                    // Circle-Circle collision
                    float dist = c1.center.Distance(c2.center);
                    bool collides = dist <= c1.radius + c2.radius;

                    if (collides && collisionPoint)
                    {
                        // Collision point is along line between centers
                        // At distance c1.radius from c1.center toward c2.center
                        if (dist > 0)
                        {
                            Point2D direction = c2.center - c1.center;
                            direction.Normalize();
                            *collisionPoint = c1.center + direction * c1.radius;
                        }
                        else
                        {
                            // Centers coincide - use c1's center
                            *collisionPoint = c1.center;
                        }
                    }
                    return collides;
                }
                //=============================================
                // Circle x Rectangle
                //=============================================
                case ShapeType::kRect:
                {
                    // Symmetry: Circle-Rect is same as Rect-Circle
                    return CheckCollision(s2, s1, collisionPoint);
                }
                //=============================================
                // Circle x Line
                //=============================================
                case ShapeType::kLine:
                {
                    const Line& l = s2.shape.line;
                    // Circle-Line collision: distance from circle center to line
                    Point2D closest = l.ClosestPoint(c1.center);
                    float d = c1.center.Distance(closest);
                    bool collides = d <= c1.radius + l.width;

                    if (collides && collisionPoint)
                    {
                        *collisionPoint = closest;
                        // std::cout << "Circle x Line Collision at point " << *collisionPoint << "\n";
                    }
                    return collides;
                }
            }
            break;
        }

        case ShapeType::kLine:
        {   
            const Line& l1 = s1.shape.line;
            switch (s2.t)
            {   //=============================================
                // Line x Line
                //=============================================
                case ShapeType::kLine:
                {
                    const Line& l2 = s2.shape.line;
                    // Line-Line collision: check if they cross
                    Point2D tmp;
                    if (l1.Crosses(l2, tmp))
                    {
                        if (collisionPoint)
                            *collisionPoint = tmp;
                        return true;
                    }
                    // If either line has width, check distances
                    if (l1.width > 0 || l2.width > 0)
                    {
                        Point2D c1 = l1.ClosestPoint(l2.p1);
                        Point2D c2 = l1.ClosestPoint(l2.p2);
                        Point2D c3 = l2.ClosestPoint(l1.p1);
                        Point2D c4 = l2.ClosestPoint(l1.p2);

                        float d1 = c1.Distance(l2.p1);
                        float d2 = c2.Distance(l2.p2);
                        float d3 = c3.Distance(l1.p1);
                        float d4 = c4.Distance(l1.p2);

                        bool collides = d1 <= l1.width || d2 <= l1.width ||
                                       d3 <= l2.width || d4 <= l2.width;

                        if (collides && collisionPoint)
                        {
                            // Use the closest point pair
                            float minDist = d1;
                            *collisionPoint = c1;
                            if (d2 < minDist) { minDist = d2; *collisionPoint = c2; }
                            if (d3 < minDist) { minDist = d3; *collisionPoint = c3; }
                            if (d4 < minDist) { *collisionPoint = c4; }
                            // std::cout << "Line x Line Collision at point " << *collisionPoint << "\n";
                        }

                        return collides;
                    }
                    return false;
                }
                //=============================================
                // Line x Circle
                //=============================================
                case ShapeType::kCircle:
                {
                    // Symmetry: Line-Circle is same as Circle-Line
                    return CheckCollision(s2, s1, collisionPoint);
                }
                //=============================================
                // Line x Rect
                //=============================================
                case ShapeType::kRect:
                {
                    // Symmetry: Line-Rect is same as Rect-Line
                    return CheckCollision(s2, s1, collisionPoint);
                }
            }
            break;
        }
    }
    return false;
}

} // namespace CMPUT350
