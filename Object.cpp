
	#include "MovableObject.h"
    #include <cstdlib>
	#include <iostream>
	#include <vector>
    #include <set>


	using namespace std;

    // All objects inside the level --Constraint: they are all rectangles.
    AObject::AObject(int width, int height, float x, float y, const char* name, int hitsUntilBreak, const char* path)
	{
		Width = width;
		Height = height;

		Coordinates =  SCoordinates(x, y);

		HitsUntilBreak = hitsUntilBreak;
		Name = name;
		Path = path; 
	}

	// Calculates the intersection between two lines that describes ball's trajectory and the four sides of this rectangle. 
	// Return the closest two intersect point from ball's location, if any.
	pair <SCoordinates, SCoordinates> AObject::CalculateNextCollision(AMovableObject Ball)
	{

		/* Saving intersect points between ball's trajectory and any side of the rectangle. Ordered by ascending distance. */
		set <SCoordinates> IntersectingPoints;
		SCoordinates InsersectPoint;

		/* Calculating first segment points in  ball trajectory */
		SCoordinates startPointTrajecSeg = SCoordinates(Ball.Coordinates.X, Ball.Coordinates.Y);
		float X, Y;
		/* We get a far away point in the trajectory */
		X = startPointTrajecSeg.X + Ball.GetDirectionUnitVector().X * MapWidth;
		Y = startPointTrajecSeg.Y + Ball.GetDirectionUnitVector().Y * MapHeight;
		SCoordinates endPointTrajecSeg = SCoordinates(X, Y);

		/* Calculating second segment points in  ball trajectory */
		SCoordinates startPointTrajecSegV2 = SCoordinates(Ball.Coordinates.X + Ball.Width, Ball.Coordinates.Y + Ball.Height);
		/* We get a far away point in the trajectory */
		X = startPointTrajecSegV2.X + Ball.GetDirectionUnitVector().X * MapWidth;
		Y = startPointTrajecSegV2.Y + Ball.GetDirectionUnitVector().Y * MapHeight;
		SCoordinates endPointTrajecSegV2 = SCoordinates(X, Y);


		/*   UPPER SIDE INTERSECTION   */
		InsersectPoint = CheckIntersect(startPointTrajecSeg, endPointTrajecSeg, Coordinates.X, Coordinates.Y, ESurfaceType::HORIZONTAL);
		if (InsersectPoint.Distance >= 0)
		{
			IntersectingPoints.insert(InsersectPoint);
		}
		InsersectPoint = CheckIntersect(startPointTrajecSegV2, endPointTrajecSegV2, Coordinates.X, Coordinates.Y, ESurfaceType::HORIZONTAL);
		if (InsersectPoint.Distance >= 0)
		{
			IntersectingPoints.insert(InsersectPoint);
		}


		/*   DOWN SIDE INTERSECTION   */
		InsersectPoint = CheckIntersect(startPointTrajecSeg, endPointTrajecSeg, Coordinates.X, Coordinates.Y + Height, ESurfaceType::HORIZONTAL);
		if (InsersectPoint.Distance >= 0)
		{
			IntersectingPoints.insert(InsersectPoint);
		}
		InsersectPoint = CheckIntersect(startPointTrajecSegV2, endPointTrajecSegV2, Coordinates.X, Coordinates.Y + Height, ESurfaceType::HORIZONTAL);
		if (InsersectPoint.Distance >= 0)
		{
			IntersectingPoints.insert(InsersectPoint);
		}


		/*   RIGHT SIDE INTERSECTION   */
		InsersectPoint = CheckIntersect(startPointTrajecSeg, endPointTrajecSeg, Coordinates.X + Width, Coordinates.Y, ESurfaceType::VERTICAL);
		if (InsersectPoint.Distance >= 0)
		{
			IntersectingPoints.insert(InsersectPoint);
		}
		InsersectPoint = CheckIntersect(startPointTrajecSegV2, endPointTrajecSegV2, Coordinates.X + Width, Coordinates.Y, ESurfaceType::VERTICAL);
		if (InsersectPoint.Distance >= 0)
		{
			IntersectingPoints.insert(InsersectPoint);
		}


		/*   LEFT SIDE INTERSECTION   */
		InsersectPoint = CheckIntersect(startPointTrajecSeg, endPointTrajecSeg, Coordinates.X, Coordinates.Y, ESurfaceType::VERTICAL);
		if (InsersectPoint.Distance >= 0)
		{
			IntersectingPoints.insert(InsersectPoint);
		}
		InsersectPoint = CheckIntersect(startPointTrajecSegV2, endPointTrajecSegV2, Coordinates.X, Coordinates.Y, ESurfaceType::VERTICAL);
		if (InsersectPoint.Distance >= 0)
		{
			IntersectingPoints.insert(InsersectPoint);
		}

		// If there isn't any intersection, we return a default value
		if (IntersectingPoints.empty())
		{
			InsersectPoint = SCoordinates(-1.0f, -1.0f);
			InsersectPoint.Distance = -1;
			return make_pair(InsersectPoint, InsersectPoint);
		}
		else
			if (IntersectingPoints.size() == 1) // only one intersection
			{
				InsersectPoint = SCoordinates(-1.0f, -1.0f);
				InsersectPoint.Distance = -1;
				return make_pair(*IntersectingPoints.begin(), InsersectPoint);
			}

		// return the two first elements of the structure, the closest ones from the Ball (set is ordered by ascending distance)
		return make_pair(*IntersectingPoints.begin(), *next(IntersectingPoints.begin()));
	}

	/*   Applies Cramer Method to determine if two segments have an intersection   */
	SCoordinates AObject::CheckIntersect(SCoordinates StartPointTrajecSeg, SCoordinates EndPointTrajecSeg, 
										float LineSideOriginX, float LineSideOriginY, ESurfaceType surfaceType)
	{
		// Stores the result
		SCoordinates IntersectionPoint;

		// Default value. Returned if there isn't intersection
		IntersectionPoint = SCoordinates(-1.0f, -1.0f);
		IntersectionPoint.Distance = -1;


		/* Sets second segment */
		SCoordinates StartPointSideSeg = SCoordinates(LineSideOriginX, LineSideOriginY);
		float width = surfaceType == ESurfaceType::HORIZONTAL? Width  : 0;
		float height = surfaceType == ESurfaceType::VERTICAL?  Height : 0;
		SCoordinates EndPointSideSeg = SCoordinates(LineSideOriginX + width, LineSideOriginY + height);

		/* Two equation system solved by Cramer's Method */
		float denom = ((EndPointSideSeg.Y - StartPointSideSeg.Y) * (EndPointTrajecSeg.X - StartPointTrajecSeg.X)) -
					  ((EndPointSideSeg.X - StartPointSideSeg.X) * (EndPointTrajecSeg.Y - StartPointTrajecSeg.Y));

		if (denom == 0) 
		{
			// They are paralel. Rare case.
			return IntersectionPoint;
		}

		float sideLineParam = 0;
		float ballTrajectParam = 0;

		ballTrajectParam = (((EndPointSideSeg.X - StartPointSideSeg.X) * (StartPointTrajecSeg.Y - StartPointSideSeg.Y)) -
						   ((EndPointSideSeg.Y - StartPointSideSeg.Y) * (StartPointTrajecSeg.X - StartPointSideSeg.X))) / denom;


		sideLineParam = (((EndPointTrajecSeg.X - StartPointTrajecSeg.X) * (StartPointTrajecSeg.Y - StartPointSideSeg.Y)) -
						((EndPointTrajecSeg.Y - StartPointTrajecSeg.Y) * (StartPointTrajecSeg.X - StartPointSideSeg.X))) / denom;

		if (ballTrajectParam < 0 || ballTrajectParam > 1 || sideLineParam < 0 || sideLineParam > 1)
		{
			return IntersectionPoint; // No intersection between both segments
		}
				
		IntersectionPoint.X = StartPointTrajecSeg.X + ballTrajectParam * (EndPointTrajecSeg.X - StartPointTrajecSeg.X);
		IntersectionPoint.Y = StartPointTrajecSeg.Y + ballTrajectParam * (EndPointTrajecSeg.Y - StartPointTrajecSeg.Y);
		IntersectionPoint.Distance = CalculateDistance(IntersectionPoint.X, StartPointTrajecSeg.X, IntersectionPoint.Y, StartPointTrajecSeg.Y);
		IntersectionPoint.SurfaceType = surfaceType;			

		return IntersectionPoint;
	}

	// Implementing the pythagorean theorem for calculating the distance between two points
	float AObject::CalculateDistance(float intersectX, float trajectoryX, float intersectY, float trajectoryY)
	{
		return sqrt((intersectX * intersectX - 2 * intersectX * trajectoryX + trajectoryX * trajectoryX) + (intersectY * intersectY - 2 * intersectY * trajectoryY + trajectoryY * trajectoryY));
	}

