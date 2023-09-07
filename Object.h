
#include "Framework.h"
#include <float.h>
#include <utility>

// Forward declaration
class AMovableObject;

enum class ESurfaceType
{
	VERTICAL,
	HORIZONTAL,
	COUNT
};

// Represents two coordinates or a vector. It also have collision info. TODO: split in two different structs.
struct SCoordinates 
{
	// Position inside the screen
	float X;
	float Y;

	// Used by the collision system to store if this point belongs to a vertical or horizontal side
	ESurfaceType SurfaceType;

	// Distance from this point to the Ball. Used by the collision system to calculate next impact point.
	float Distance = -1;

	// Stores object index. Value -1 means the player.
	int ObjectIndex = -2;

	// To diferenciate block hit points from wall hit points
	bool IsBlock = false;

	SCoordinates(){}
	SCoordinates(float x, float y)
	{
		X = x;
		Y = y;
	}
	SCoordinates(float x, float y, float distance, ESurfaceType surface)
	{
		X = x;
		Y = y;
		Distance = distance;
		SurfaceType = surface;
	}

	// used in the set structure during each insertion for maintain ascending order
	bool operator< (const SCoordinates& e) const
	{
		return Distance < e.Distance;
	}
	int GetX() { return (int)X; }
	int GetY() { return (int)Y; }

};


class AObject 
{

public:

	int  Width = 0, Height = 0; // Sizes of this rectangle
	int  HitsUntilBreak = -1;	// Negative means unbreakable
	const char* Name;			// For debuggin purposes

	SCoordinates Coordinates;

	Sprite* Sprite;

	const char* Path;           // used to get the broken sprites

	AObject() {};
	AObject(int width, int height, float x, float y, const char* name = "Default", int hitsUntilBreak = -1, const char* path ="");

	/* Calculate lines from two ball's locations, simulating the trajectory. Return two intersection points */
	std::pair <SCoordinates, SCoordinates> CalculateNextCollision(AMovableObject Ball);

	/* Applies Cramer Method to determine if two segments have an intersection */
	SCoordinates CheckIntersect(SCoordinates StartPointTrajecSeg, SCoordinates EndPointTrajecSeg, 
								float lineSideOriginX, float lineSideOriginY, ESurfaceType surfaceType);

	/* Implementing the pythagorean theorem for calculating the distance between two points */
	float CalculateDistance(float intersectX, float trajectoryX, float intersectY, float trajectoryY);

	int GetX() { return (int)Coordinates.X; }
	int GetY() { return (int)Coordinates.Y; }

};

