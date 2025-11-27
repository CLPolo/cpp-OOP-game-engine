#ifndef ENEMY_H
#define ENEMY_H

#ifndef M_PI_4
#define M_PI_4
#endif

#include "CollisionObject.h"

using namespace CMPUT350;

class Enemy : public CollisionObject
{
public:
    // Enemy
    Enemy(GameObject *mParent, Point2D loc);

	void Rotate(float angle);
	void Move(Point2D dir);
    void SetMoveDir(Point2D dir);
    bool IsOutsideBase();

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
private:
    void UpdateBounds();
    const float ENEMY_SIZE = 30.f;
    const float LINE_WIDTH = 10.f;
    const RGBColor C_BODY = Colors::magenta;
    const RGBColor C_LINE = Colors::red;
    Rect mBounds;
    std::vector<Shape> mShapes;
    Point2D mPos;


    Point2D mFacingDir;
    Point2D mMoveDir;
    float mSpeed;
    bool isAlive;
    Line mBody;
    Line mLine1, mLine2, mLine3;
    // sf::Clock mClock;

    GameObject* mParent; 
    GameContext* mContext = nullptr;
};


#endif