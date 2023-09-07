#include "Object.h"

#pragma once
class AMovableObject : public AObject
{
public:
	float Speed = 0;
	bool bIsMovingLeft = false;
	bool bIsMovingRight = false;

private:
	// Stores the direction unit vector
	SCoordinates DirectionUnitVector;
	void ApplyModule();

public:
	AMovableObject() {};
	AMovableObject(int width, int height, float x, float y, float speed,const char* name = "Default", int hitsToBreak = -1, const char* path = "");

	inline SCoordinates GetDirectionUnitVector() { return DirectionUnitVector; }
	void SetDirectionUnitVector(float firstPointX, float firstPointY, float endPointX, float endpointY);
	
	void AddOffsetDirUnitVector(float offsetX, float offsetY);

	// Move the object along the X axis
	void HorizontalMove(float OutLimitsOffset = 0);

	// Move the object in any axis simultaneously
	void FreeMove();

	/* Bounce object after collide with something */
	void Bounce(ESurfaceType surfaceType);
};