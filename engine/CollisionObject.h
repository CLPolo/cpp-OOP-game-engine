#ifndef COLLISION_OBJECT_H
#define COLLISION_OBJECT_H

#include <vector>
#include <memory>

namespace CMPUT350 {
    class GameObject;
}

#include "GameObject.h"
#include "MathUtil.h"

namespace CMPUT350 {

enum class ShapeType {
    kLine, kCircle, kRect
};

union ShapeUnion {
    ShapeUnion(Line l) : line(l) {}
    ShapeUnion(Circle c) : circle(c) {}
    ShapeUnion(Rect r) : rect(r) {}
    ~ShapeUnion() {} 
    Line line;
    Circle circle;
    Rect rect;
};

struct Shape {
    Shape(const Line &l) : shape(l), t(ShapeType::kLine) {}
    Shape(const Circle &c) : shape(c), t(ShapeType::kCircle) {}
    Shape(const Rect &r) : shape(r), t(ShapeType::kRect) {}
    ~Shape() {} // Destructor required because union has destructor
    ShapeUnion shape;
    ShapeType t;
};


// Check collision and return collision point (when applicable)
bool CheckCollision(const Shape& s1, const Shape& s2, Point2D *collisionPoint);

// Check collision between two shapes
inline bool CheckCollision(const Shape& s1, const Shape& s2)
{
    return CheckCollision(s1, s2, nullptr);
}

class CollisionObject : public GameObject {
public:
    // Indicates that object does not move. Will only have collisions checked
    // against non-static objects.
    virtual bool IsStatic() const = 0;

    virtual void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) = 0;

    virtual void CollisionEnter(const std::shared_ptr<CollisionObject> &obj, const Point2D &collisionPoint);

    virtual const Rect &GetBounds() = 0;

    // Return the internal shapes in the object. Shapes can be:
    // lines, points/circles, or axis-aligned rectangles
    virtual const std::vector<Shape> &GetShapes() = 0;
};

}

#endif 