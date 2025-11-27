#ifndef PLAYER_H
#define PLAYER_H

#ifndef M_PI_4
#define M_PI_4 (M_PI/4.0)
#endif

#include "CollisionObject.h"

using namespace CMPUT350;

class Player : public CollisionObject
{
public:
	// Player
	Player(Point2D loc);
	void Rotate(float angle);
	void Move(Point2D dir);

	// GameObject
	void Update(GameContext *context);
	void LateUpdate(GameContext *context);
	void RenderBackground(GameContext *contextrender);
	void RenderForeground(GameContext *contextrender);
	bool HandleKeyEvent(GameContext *context, char key);
	bool IsAlive();
	void Kill();

	// CollisionObject interface
	bool IsStatic() const override { return false; }
	void CollisionEnter(const std::shared_ptr<CollisionObject> &obj) override;
	const std::vector<Shape> &GetShapes() override;
	const Rect &GetBounds() override;

	// Position accessor for camera tracking
	Point2D GetPosition() const { return mPos; }

private:
	void _UpdateBounds();
	void _UpdateShapes();
	void _DebugOut();

	const float PLAYER_SIZE = 30.f;
	const float SPEED_INTERVAL = 5.f;
	const float MAX_SPEED = 3.f;
	const float TURRET_LENGTH = 20.f;
	const float LINE_WIDTH = 10.f;
	const float ROTATION_STEP = M_PI_4;

	const RGBColor C_TREAD = Colors::white;
	const RGBColor C_COCKPIT = Colors::green;
	const RGBColor C_TURRET = Colors::gray;

	Rect mBounds;
	std::vector<Shape> mShapes;
	Line mTurret, mTreadR, mTreadL, mCockpit;
	Point2D mPos;
	Point2D mFacingDir;
	Point2D mTurretDir;
	Point2D mMoveDir;
	float mSpeed;
	bool isAlive;

	// Shot delay tracking (5 frames minimum between shots)
	int mFramesSinceLastShot;
	const int MIN_SHOT_DELAY = 5;
};

#endif
