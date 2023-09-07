
#include "MovableObject.h"
#include <math.h> 

AMovableObject::AMovableObject(int width, int height, float x, float y, float speed, const char* name, int hitsUntilBreak, const char* path)
	    : AObject(width, height, x, y, name, hitsUntilBreak, path)
	{
		Speed = speed;
	}

	void AMovableObject::HorizontalMove(float OutLimitsOffset)
	{
		if (bIsMovingLeft && Coordinates.X > 0 + OutLimitsOffset)
		{
			Coordinates.X -= Speed;
		}

		else
			if (bIsMovingRight && Coordinates.X < MapWidth - Width - OutLimitsOffset) {
				Coordinates.X += Speed;
			}

	}

	void AMovableObject::ApplyModule()
	{
		double module = sqrt(DirectionUnitVector.X * DirectionUnitVector.X + DirectionUnitVector.Y * DirectionUnitVector.Y);
		DirectionUnitVector = SCoordinates(DirectionUnitVector.X / module, DirectionUnitVector.Y / module);
	}

	void AMovableObject::SetDirectionUnitVector(float firstPointX, float firstPointY, float endPointX, float endpointY)
	{
		SCoordinates directionVector = SCoordinates(endPointX - firstPointX, endpointY - firstPointY);
		DirectionUnitVector = SCoordinates(directionVector.X, directionVector.Y);
		ApplyModule();
	}


	void AMovableObject::AddOffsetDirUnitVector(float offsetX, float offsetY)
	{

		if (DirectionUnitVector.X + offsetX > 1)
			DirectionUnitVector.X = 1;
		else
		if (DirectionUnitVector.X + offsetX < -1)
			DirectionUnitVector.X += -1;
		else
			DirectionUnitVector.X += offsetX;

		if (DirectionUnitVector.Y + offsetY > 1)
			DirectionUnitVector.Y = 1;
		else
			if (DirectionUnitVector.Y + offsetY < -1)
				DirectionUnitVector.Y += -1;
			else
				DirectionUnitVector.Y += offsetY;

		ApplyModule();
	}

	/* Moves in any direction */
	void AMovableObject::FreeMove()
	{
		Coordinates.X += Speed * DirectionUnitVector.X;
		Coordinates.Y += Speed * DirectionUnitVector.Y;
	}

	/* Mirrors the direction unit vector depending on the hit surface */
	void AMovableObject::Bounce(ESurfaceType surfaceType)
	{
		DirectionUnitVector = (surfaceType == ESurfaceType::VERTICAL) ?
			SCoordinates(- DirectionUnitVector.X, DirectionUnitVector.Y)
			: SCoordinates(DirectionUnitVector.X, -DirectionUnitVector.Y);
	}