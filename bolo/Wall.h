#ifndef WALL_H
#define WALL_H

#include "CollisionObject.h"

using namespace CMPUT350;

enum tWallDirection {
	kHorizontal,
	kVertical
};

class Wall : public CollisionObject
{
public:
	Wall(Point2D origin, float length, tWallDirection dir, float width);

	// CollisionObject interface
	bool IsStatic() const override { return true; }
	void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override {};
	const std::vector<Shape> &GetShapes() override { return mShapes; }
    const Rect &GetBounds() override { return mBounds; }

	void RenderForeground(GameContext *context) override;

private:
	Point2D origin;
	Line shape;
	float length;
	float width;
	Rect mBounds;
	std::vector<Shape> mShapes;
};

#endif
