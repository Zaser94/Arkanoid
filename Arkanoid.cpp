

	#include "Framework.h"
	#include "MovableObject.h"
	#include "NegativeModifier.h"
	#include "PositiveModifier.h"
	#include <string>
	#include <algorithm>
	#include <iostream>
	#include <vector>
    #include <set>
	#include <memory>

	using namespace std;

	/***********************************************************************
	*                                STRUCTURES                            *
	************************************************************************/
	enum class GameState {
		INITIAL,
		THROWING,
		PLAYING,
		DEFEAT,
		VICTORY,
		COUNT
	};

	struct strModifier 

	{   
		/* If I save the Parent class there is a lose of info when I cast to the child, no matter if I'm storaging the pointer,
		   So, I will do a workaround,storing also a pointer to the childs, no time for a better solution */
		AMovableObject* Modifier = 0; 
		ANegativeModifier* NegativeModifier = 0;
		APositiveModifier* PositiveModifier = 0;
		unsigned int Duration = 10000;
		unsigned int TimeToStopEffect = 0;
		pair<SCoordinates, SCoordinates> CollisionPoints;
		bool IsPlayerCollisionDone = false;
	};

	class Arkanoid : public Framework {

	public:

	/***********************************************************************
	*                            GAME PROPERTIES                           *
	************************************************************************/

		
     /*     Game Flow       */
	 GameState GameState;
	 SCoordinates FirstPositionAfterClick;     // Stores the first position where the ball must move after the player press left click
	 bool GameHasStart = false;

	 /*   Player   */
	 AMovableObject Player;
	 int PlayerWidth = 100;
	 int PlayerHeight = 40;
	 float PlayerSpeed = 4.0f;
	 float dirOffsetWhenHitBall = 0.3f;
	 float   PlayerMapHeighOffet = 100.f;          // Player Y offset from MapHeight
	 float WallOffet = 50;                     // used to determine horizontal move limits
	 int nLives = 3;
	 vector <AObject*> Lives;

	 /* Ball */
	 AMovableObject Ball;
	 float BallSpeed = 4;

	 /* Blocks */
	 int BlockWidth = 80;
	 int BlockHeight = 40;
	 float BlockSpeed = 0.05f;

	 /* Modifiers */
	 vector <strModifier*> Modifiers;  // Storage the required variables for executing all the logic related to modifiers
	 unsigned int TimeBeforeSpawnNextMod = 10000; 
	 unsigned int TimeSpawnIncrement = 10000;		
	 int nextPositiveMod = 0;     
	 int nextNegativeMod = 0;		
	 bool IsNextModPositive = true;
	 float ModSpeed = 1.5;
	 int LongPlayerIncrement = 50;
	 int BallSpeedIncrement = 3;
	 int SlowPlayerDecrement = 2;


	 /* Collision System */
	 vector <AMovableObject*> Blocks;
	 vector <AObject*> Walls;
	 pair<SCoordinates, SCoordinates> NextBallCollisionPoints;
	 float	BallErrorTolerance = 5.0f;    // Max. difference allowed to determine if the ball reach the intersection


	/* Player Animation */
	 vector <Sprite*> PlayerSprites;
	 string playerSpritePath = "data\\PlayerSprite";
	 short playerSpriteIndex = 1;

	 /* End Screens */
	 Sprite* VictoryScreen = 0;
	 Sprite* DefeatScreen = 0;
	 int EndScreenWidth = 328;
	 int EndScreenHeight = 159;

	 /* Throwing Phase */
	 unsigned int ThrowingPhaseDuration = 50; //milliseconds
	 unsigned int ThrowingPhaseStartTime = 0;

	 /* Debug */
	 bool IsDebugActive = 0;
	 Sprite* DebugPoint = 0;
	 unsigned int TrappedCount = 0;
	 SCoordinates LastBallPosistion = SCoordinates(0,0);

	 /***********************************************************************
	 *                               FUNCTIONS                              *
	 ************************************************************************/

	 AObject* SetUpObject(const char* path, int width, int height, float x, 
						 float y, const char* name ="Default", int hitsToBreak = -1)
	 {
		 AObject* object;
		 object = new AObject(width, height, x, y, name, hitsToBreak);
		 object->Sprite = createSprite(path);
		 setSpriteSize(object->Sprite, object->Width, object->Height);
		 return object;
	 }

	 AMovableObject* SetUpMovableObject(const char* path, int width, int height, float x,
										float y,float speed ,const char* name = "Default", int hitsToBreak = -1)
	 {
		 AMovableObject* object;
		 object = new AMovableObject(width, height, x, y, speed, name, hitsToBreak, path);
		 object->Sprite = createSprite(path);
		 setSpriteSize(object->Sprite, object->Width, object->Height);
		 return object;
	 }

	 void SetUpLevelWalls(const char* path,int width, int offset)
	 {
		 int mapHeight = MapHeight;
		 int mapWidth = MapWidth;
		 Walls.push_back(SetUpObject(path, mapWidth + offset, width, (float) - offset / 2, 0, "NorthWall"));
		 Walls.push_back(SetUpObject(path, width, mapHeight + offset,0 , (float)-offset/2, "WestWall"));
		 Walls.push_back(SetUpObject(path, width, mapHeight + offset, (float)mapWidth - width, (float)-offset/2, "EastWall"));

		 // Workaround to fix a collision bug at the corners when player requires to bounce twice in the same frame
		 Walls.push_back(SetUpObject(path, 60, 60 , mapWidth-60,0 , "EastCorner"));
		 Walls.push_back(SetUpObject(path, 60,60 , 0, 0, "WestCorner"));
	 }

	 void SetUpBlocksInRow(const char* path,int width,int height, float widthInMap, float heightInMap, int numObjects, int hitsToBreak, float speed)
	 {
		 for (int i = 0; i < numObjects; i++)
		 {
			 AMovableObject* block = SetUpMovableObject(path, width, height, widthInMap + i * width, heightInMap, speed, "Block-" + i, hitsToBreak);
			 block->SetDirectionUnitVector(0, 0, 0, 1);
			 Blocks.push_back(block);
		 }

	 }

	 void SetUpLives(const char* path, int numlives)
	 {
		 for (int i = 0; i < numlives; i++)
		 {
			 Lives.push_back(SetUpObject(path, 30, 30, (float)60+(i*50), MapHeight - PlayerMapHeighOffet+45.0f, "Live " + i));
		 }
	 }

	 // Checks if two floats are almost the same 
	 bool NearlyEqual(float n1, float n2, float tolerance)
	 {
		 return abs(n1 - n2) < tolerance;
	 }

	 // The collision system pre-calculates two intersection points. This function returns true whenever the object is close to one of this points.
	 pair<bool, SCoordinates> ObjectReachNextIntersection(AMovableObject object, pair<SCoordinates, SCoordinates> collisionPoints, float errorTolerance)
	 {
		 if ((NearlyEqual(object.Coordinates.X, collisionPoints.first.X, errorTolerance) &&
			 NearlyEqual(object.Coordinates.Y, collisionPoints.first.Y, errorTolerance)) ||
			 (NearlyEqual(object.Coordinates.X + object.Width, collisionPoints.first.X, errorTolerance) &&
				 NearlyEqual(object.Coordinates.Y + object.Height, collisionPoints.first.Y, errorTolerance)))
		 {
			 return make_pair(true, collisionPoints.first);
		 }
		 else
			 if ((NearlyEqual(object.Coordinates.X, collisionPoints.second.X, errorTolerance) &&
				 NearlyEqual(object.Coordinates.Y, collisionPoints.second.Y, errorTolerance)) ||
				 (NearlyEqual(object.Coordinates.X + object.Width, collisionPoints.second.X, errorTolerance) &&
					 NearlyEqual(object.Coordinates.Y + object.Height, collisionPoints.second.Y, errorTolerance)))
			 {
				 return make_pair(true, collisionPoints.second);
			 }
			 else
				 return make_pair(false, SCoordinates(0, 0));
	 }



	 // Check if the ball collides with the player or a block. 
	 pair<SCoordinates, SCoordinates> CheckingBallCollisions()
	 {
		 // Saving intersect points between ball's trajectory and any rectangle. Ascending ordered by distance.
		 set <SCoordinates> IntersectingPoints;

		 // Check player's collisions.
		pair<SCoordinates, SCoordinates> nextCollision = Player.CalculateNextCollision(Ball);

		if (nextCollision.first.Distance >= 0)    // if distance is valid 
		{
			nextCollision.first.ObjectIndex = -1; // Value -1 means the player
			IntersectingPoints.insert(nextCollision.first);
		}
		if (nextCollision.second.Distance >= 0)
		{
			nextCollision.second.ObjectIndex = -1;
			IntersectingPoints.insert(nextCollision.second);
		}


		 // Check collision agains  walls
		 for (int i = 0; i < Walls.size(); i++)
		 {
			 pair<SCoordinates, SCoordinates> nextCollision = Walls[i]->CalculateNextCollision(Ball);
			 if (nextCollision.first.Distance >= 0) // if distance is valid
			 {
				 nextCollision.first.ObjectIndex = i;
				 IntersectingPoints.insert(nextCollision.first);
			 }
			 if (nextCollision.second.Distance >= 0) // if distance is valid
			 {
				 nextCollision.second.ObjectIndex = i;
				 IntersectingPoints.insert(nextCollision.second);
			 }
		 }

		 // Check collision agains blocks
		 for (int i = 0; i < Blocks.size(); i++)
		 {
			 pair<SCoordinates, SCoordinates> nextCollision = Blocks[i]->CalculateNextCollision(Ball);
			 if (nextCollision.first.Distance >= 0) // if distance is valid
			 {
				 nextCollision.first.ObjectIndex = i;
				 nextCollision.first.IsBlock = true;
				 IntersectingPoints.insert(nextCollision.first);
			 }
			 if (nextCollision.second.Distance >= 0) // if distance is valid
			 {
				 nextCollision.second.ObjectIndex = i;
				 nextCollision.first.IsBlock = true;
				 IntersectingPoints.insert(nextCollision.second);
			 }
		 }

		 if (IntersectingPoints.size() == 0)
		 {   /* No collisions detected */
			 return make_pair(SCoordinates(0, 0), SCoordinates(0, 0));
		 }
		 if (IntersectingPoints.size() == 1)
		 {
			 return make_pair(*IntersectingPoints.begin(), SCoordinates(0, 0));
		 }
		 else
		 {
			 return make_pair(*IntersectingPoints.begin(), *next(IntersectingPoints.begin()));
		 }

	 }

	 // Check if any modifier collides with the player or a wall. Saves the intersection points in the modifier estructure.
	 // TODO: join ball collision system with this new system to avoid code redundance.
	 void CheckingModifiersCollisions()
	 {
		 for (int i = 0; i < Modifiers.size(); i++)
		 {
			 // Saving intersect points between modifier's trajectory and player/walls. Ordered by ascending distance
			 set <SCoordinates> IntersectingPoints;

			 /* Check player's collisions */
			 pair<SCoordinates, SCoordinates> nextCollision = Player.CalculateNextCollision(*Modifiers[i]->Modifier);

			 if (nextCollision.first.Distance >= 0)    
			 {
				 nextCollision.first.ObjectIndex = -1; // Value -1 means the player
				 IntersectingPoints.insert(nextCollision.first);
			 }
			 if (nextCollision.second.Distance >= 0)
			 {
				 nextCollision.second.ObjectIndex = -1;
				 IntersectingPoints.insert(nextCollision.second);
			 }

			 /*  Check collision agains each wall  */
			 for (int j = 0; j < Walls.size(); j++)
			 {
				 pair<SCoordinates, SCoordinates> nextCollision = Walls[j]->CalculateNextCollision(*Modifiers[i]->Modifier);
				 if (nextCollision.first.Distance >= 0) 
				 {
					 nextCollision.first.ObjectIndex = j;
					 IntersectingPoints.insert(nextCollision.first);
				 }
				 if (nextCollision.second.Distance >= 0) 
				 {
					 nextCollision.second.ObjectIndex = j;
					 IntersectingPoints.insert(nextCollision.second);
				 }
			 }

			 if (IntersectingPoints.size() == 0)
			 {   /* No collisions detected */
				 Modifiers[i]->CollisionPoints = make_pair(SCoordinates(0, 0), SCoordinates(0, 0));
			 }
			 else
			 if (IntersectingPoints.size() == 1)
			 {
				 Modifiers[i]->CollisionPoints = make_pair(*IntersectingPoints.begin(), SCoordinates(0, 0));
			 }
			 else if (IntersectingPoints.size()>1)
			 {
				 Modifiers[i]->CollisionPoints = make_pair(*IntersectingPoints.begin(), *next(IntersectingPoints.begin()));
			 }

		 }

	 }

	 void CheckingBlocksVsPlayerCollision()
	 {
		 for (int i = 0; i < Blocks.size(); i++)
		 {
			 // Until they blocks reach player's Y height is not neccesary check nothing
			 if (Blocks[i]->Coordinates.Y + Blocks[i]->Height > MapHeight - PlayerMapHeighOffet-20)
			 {
				if((Player.Coordinates.X <= Blocks[i]->Coordinates.X &&  // top player segment vs rectangle bottom left corner
					Blocks[i]->Coordinates.X <= Player.Coordinates.X + Player.Width &&
					Player.Coordinates.Y <= Blocks[i]->Coordinates.Y + Blocks[i]->Height &&
					Blocks[i]->Coordinates.Y + Blocks[i]->Height <= Player.Coordinates.Y + Player.Height)
					||
					(Player.Coordinates.X <= Blocks[i]->Coordinates.X + Blocks[i]->Width &&  // top player segment vs rectangle bottom right corner
					Blocks[i]->Coordinates.X + Blocks[i]->Width <= Player.Coordinates.X + Player.Width &&
					Player.Coordinates.Y <= Blocks[i]->Coordinates.Y + Blocks[i]->Height && 
					Blocks[i]->Coordinates.Y + Blocks[i]->Height <= Player.Coordinates.Y+Player.Height))
				 {
					//A block hits the player!
					GameState = GameState::DEFEAT;
				 }

				if (Blocks[i]->Coordinates.Y + Blocks[i]->Height > MapHeight)
				{
					//A block reaches the bottom!
					GameState = GameState::DEFEAT;
				}
			 }
		 }
	 }

	 void ExecuteBallHitLogic(SCoordinates hitPoint)
	 {
		 Ball.Bounce(hitPoint.SurfaceType);

		 // If the ball hit the player
		 if (hitPoint.ObjectIndex == -1)
		 {
			 // Changes ball trajectory if the player is moving.
			 if (Player.bIsMovingLeft) Ball.AddOffsetDirUnitVector(-dirOffsetWhenHitBall, -dirOffsetWhenHitBall);
			 else
				 if (Player.bIsMovingRight) Ball.AddOffsetDirUnitVector(dirOffsetWhenHitBall, dirOffsetWhenHitBall);
		 }
		 else
			 if (hitPoint.IsBlock && hitPoint.ObjectIndex >= 0)
			 {
				 AObject* hitBlock = Blocks[hitPoint.ObjectIndex];
				 if (hitBlock->HitsUntilBreak > 0)
				 {
					 hitBlock->HitsUntilBreak -= 1; /* block lose one life */
					 if (hitBlock->HitsUntilBreak == 1)
					 {
						 // showing broken sprite
						 std::string buff(Blocks[hitPoint.ObjectIndex]->Path);
						 string newPath = buff.substr(0, 14);
						 Blocks[hitPoint.ObjectIndex]->Sprite = createSprite(newPath.append("Broken.png").c_str());
						 setSpriteSize(Blocks[hitPoint.ObjectIndex]->Sprite, Blocks[hitPoint.ObjectIndex]->Width, Blocks[hitPoint.ObjectIndex]->Height);
					 }
					 if (hitBlock->HitsUntilBreak == 0)
					 {
						 Blocks.erase(Blocks.begin() + hitPoint.ObjectIndex);

						 if (Blocks.empty()) 
						 {
							 GameState = GameState::VICTORY;
							 showCursor(true);
						 }
					 }
				 }
			 }
	 }

	 static unique_ptr<AMovableObject> GetThing()
	 {
		 return make_unique<APositiveModifier>();
	 }
	 static unique_ptr<AMovableObject> GetThing2()
	 {
		 return make_unique<ANegativeModifier>();
	 }

	 void ExecuteModifierLogic(strModifier* strModifier,SCoordinates hitPoint)
	 {
		 if (hitPoint.ObjectIndex == -1 && !strModifier->IsPlayerCollisionDone) // hit point is the player
		 {
			 strModifier->IsPlayerCollisionDone = true; // Only collides once

			 //if (APositiveModifier* positive = (APositiveModifier*) strModifier->Modifier) // if the modifier is positive
			 if(strModifier->PositiveModifier)
			 {
				 switch (strModifier->PositiveModifier->GetType())
				 {
				 case EPositiveModifier::EXTRAWALL: 
				 {
					 AObject* object = SetUpObject("Data\\Wall3.png", MapWidth+100, 40, -50,760, "ExtraWall");
					 Walls.push_back(object);
					 strModifier->TimeToStopEffect = getTickCount() + strModifier->Duration;
				 }
					 break;
				 case EPositiveModifier::MAGNET: ResetBallPosition();
					 break;
				 case EPositiveModifier::LONGPLAYER:
				 {
					 Player.Width += LongPlayerIncrement;
					 setSpriteSize(Player.Sprite, Player.Width, PlayerHeight);
					 strModifier->TimeToStopEffect = getTickCount() + strModifier->Duration;
				 }
					 break;
				 default:
					 break;
				 }
			 }
			 else
			 //if (ANegativeModifier* negative = (ANegativeModifier*)strModifier->Modifier) // if the modifier is negative
			 if(strModifier->NegativeModifier)
			 {
				 switch (strModifier->NegativeModifier->GetType())
				 {
				 case ENegativeModifier::DAMAGE: Lives.pop_back();
					 break;
				 case ENegativeModifier::FASTBALL: 
				 {
					 Ball.Speed += BallSpeedIncrement;
					 strModifier->TimeToStopEffect = getTickCount() + strModifier->Duration;
				 }
					 break;
				 case ENegativeModifier::SLOWPLAYER:
				 {
					 Player.Speed -= SlowPlayerDecrement;
					 strModifier->TimeToStopEffect = getTickCount() + strModifier->Duration;
				 }
					 break;
				 default:
					 break;
				 }
			 }
		 }
		 else
		 {
			 strModifier->Modifier->Bounce(hitPoint.SurfaceType); // hit point is a wall
		 }

	 }

	 /* When the system timer gets bigger than the modifier effect time this logic happens */
	 void FinishExecuteModifierLogic(strModifier* strModifier)
	 {

		 if(strModifier->PositiveModifier)
		 {
			 switch (strModifier->PositiveModifier->GetType())
			 {
			 case EPositiveModifier::EXTRAWALL: Walls.pop_back();
				 break;
			 case EPositiveModifier::LONGPLAYER:
			 {
				 Player.Width -= LongPlayerIncrement;
				 setSpriteSize(Player.Sprite, Player.Width, PlayerHeight);
			 }
				 break;
			 default:
				 break;
			 }
		 }
		 else
			if(strModifier->NegativeModifier)
			{
				switch (strModifier->NegativeModifier->GetType())
				{
				case ENegativeModifier::FASTBALL: Ball.Speed = BallSpeed;
					break;
				case ENegativeModifier::SLOWPLAYER: Player.Speed = PlayerSpeed;
					break;
				default:
					break;
				}
			}

		 // finding and deleting the modifier info, not needed no more
		 Modifiers.erase(remove(Modifiers.begin(), Modifiers.end(), strModifier), Modifiers.end());
	 }

	 /* Draws all level objects */
	 void DrawingObjects()
	 {
		 // Player
		 UpdatePlayerSprite();

		 // Ball
		 drawSprite(Ball.Sprite, Ball.GetX(), Ball.GetY());
		 if (IsDebugActive && GameState == GameState::PLAYING)
		 {
			 drawSprite(DebugPoint, NextBallCollisionPoints.first.GetX(), NextBallCollisionPoints.first.GetY());
			 drawSprite(DebugPoint, NextBallCollisionPoints.second.GetX(), NextBallCollisionPoints.second.GetY());
		 }

		 // Blocks
		 for (int i = 0; i < Blocks.size(); i++)
		 {
			 drawSprite(Blocks[i]->Sprite, Blocks[i]->GetX(), Blocks[i]->GetY());
		 }	

		 //Modifiers
		 for (int i = 0; i < Modifiers.size(); i++)
		 {
			 if (!Modifiers[i]->IsPlayerCollisionDone)
			 {
				 drawSprite(Modifiers[i]->Modifier->Sprite, Modifiers[i]->Modifier->GetX(),
					 Modifiers[i]->Modifier->GetY());
				 if (IsDebugActive)
				 {
					 drawSprite(DebugPoint, Modifiers[i]->CollisionPoints.first.GetX(),
						 Modifiers[i]->CollisionPoints.first.GetY());
					 drawSprite(DebugPoint, Modifiers[i]->CollisionPoints.second.GetX(),
						 Modifiers[i]->CollisionPoints.second.GetY());
				 }
			 }
		 }

		 // Walls
		 for (int i = 0; i < Walls.size(); i++)
		 {
			 drawSprite(Walls[i]->Sprite, Walls[i]->GetX(), Walls[i]->GetY());
		 }

		 //Lives
		 for (int i = 0; i < Lives.size(); i++)
		 {
			 drawSprite(Lives[i]->Sprite, Lives[i]->GetX(), Lives[i]->GetY());
		 }

	 }

	 void UpdatePlayerSprite()
	 {
		 //Changing sprite in every  frames.
		 if (getTickCount() % 300 > 150)
		 {
			 Player.Sprite = PlayerSprites[playerSpriteIndex];
			 setSpriteSize(Player.Sprite, Player.Width, Player.Height);

			 // updating index to show next sprite animation
			 playerSpriteIndex = (playerSpriteIndex < PlayerSprites.size() - 1) ? playerSpriteIndex + 1 : 0;
		 }

		 drawSprite(Player.Sprite, Player.GetX(), Player.GetY());
	 }

	 void ThrowBall() 
	 {
		 // Updating Ball Direction
		 Ball.SetDirectionUnitVector(Ball.Coordinates.X, Ball.Coordinates.Y, FirstPositionAfterClick.X, FirstPositionAfterClick.Y);
		 Ball.Speed = BallSpeed;
		 showCursor(false);
		 GameState = GameState::THROWING;

	 }

	 void SpawnNextMod() 
	 {
		 if (IsNextModPositive) 
		 {
			 APositiveModifier* newModifier = new APositiveModifier(50, 50, MapWidth / 2, MapHeight / 2, ModSpeed, static_cast<EPositiveModifier>(nextPositiveMod));
			 newModifier->Sprite = createSprite(newModifier->GetPath());
			 setSpriteSize(newModifier->Sprite, newModifier->Width, newModifier->Height);
			 strModifier* stMod = new strModifier;
			 stMod->Modifier = newModifier;
			 stMod->PositiveModifier = newModifier;
			 Modifiers.push_back(stMod);

			 nextPositiveMod++;
			 if (nextPositiveMod == static_cast<int>(EPositiveModifier::COUNT))
			 {
				 nextPositiveMod = 0;
			 }

		 }
		 else
		 {
			 ANegativeModifier* newModifier = new ANegativeModifier(50, 50, MapWidth / 2, MapHeight / 2, ModSpeed, static_cast<ENegativeModifier>(nextNegativeMod));
			 newModifier->Sprite = createSprite(newModifier->GetPath());
			 setSpriteSize(newModifier->Sprite, newModifier->Width, newModifier->Height);
			 strModifier* stMod = new strModifier;
			 stMod->Modifier = newModifier;
			 stMod->NegativeModifier = newModifier;
			 Modifiers.push_back(stMod);

			 nextNegativeMod++;
			 if (nextNegativeMod == static_cast<int>(ENegativeModifier::COUNT))
			 {
				 nextNegativeMod = 0;
			 }
				 
		 }
	 }

	 void ResetBallPosition()
	 {
		Ball.Coordinates.X = Player.Coordinates.X + 40;
 		Ball.Coordinates.Y = Player.Coordinates.Y - 20;
 		Ball.Speed = Player.Speed;
 		GameState = GameState::INITIAL;
 		showCursor(true);
	 }

	 /***********************************************************************
	 *                               PRE INIT                               *
	 ************************************************************************/

	void PreInit(int& width, int& height, bool& fullscreen)
	{
		width = MapWidth;
		height = MapHeight;
		fullscreen = false;
	}

	/***********************************************************************
	*                                  INIT                                *
	************************************************************************/

	bool Init() {

		/* Cleaning structures */
		Blocks.clear();
		Lives.clear();
		Walls.clear();
		Modifiers.clear();
		PlayerSprites.clear();

		// Reset modifier vars
		TimeBeforeSpawnNextMod = 10000 +getTickCount(); // first modifier after 20s
		TimeSpawnIncrement = 7000;      // new modifier every 7s
		nextPositiveMod = 0;
		nextNegativeMod = 0;
		IsNextModPositive = true;

		/* Player initialization */
		Player = AMovableObject(PlayerWidth, PlayerHeight, MapWidth/2.0f - PlayerWidth/2.0f, MapHeight- PlayerMapHeighOffet, PlayerSpeed,"Player");
		Player.Sprite = createSprite("data\\PlayerSprite1.png");
		setSpriteSize(Player.Sprite, Player.Width, Player.Height);

		SetUpLives("data\\Live.png", nLives);

		for (int i = 1; i < 3; i++)
		{  
			PlayerSprites.push_back(createSprite((playerSpritePath + to_string(i) + ".png").c_str()));  // Animations
		}


		/* Ball initialization */
		Ball = AMovableObject(20, 20, Player.Coordinates.X+40, Player.Coordinates.Y-20, PlayerSpeed, "Ball");
		Ball.Sprite = createSprite("data\\Ball.png");
		setSpriteSize(Ball.Sprite, Ball.Width, Ball.Height);

		/***********************************************************************
		*                              LEVEL BLOCKS                            *
		************************************************************************/
		// Wall
		SetUpLevelWalls("data\\Wall.png", WallOffet, 100);

		// First row
		SetUpBlocksInRow("data\\1HitBlock.png", BlockWidth,BlockHeight, 100,500, 5, 1, BlockSpeed);
		
		// Second row
		SetUpBlocksInRow("data\\2HitBlock.png", BlockWidth, BlockHeight, 40, 400, 1, 2, BlockSpeed);
		SetUpBlocksInRow("data\\2HitBlock.png", BlockWidth, BlockHeight, 255, 400, 1, 2, BlockSpeed);
		SetUpBlocksInRow("data\\2HitBlock.png", BlockWidth, BlockHeight, 480, 400, 1, 2, BlockSpeed);
					
		// Third row
		SetUpBlocksInRow("data\\3HitBlock.png", BlockWidth, BlockHeight, 140, 300, 4, 3, BlockSpeed);

		// Forth row
		SetUpBlocksInRow("data\\4HitBlock.png", BlockWidth, BlockHeight, 100, 200, 5, 4, BlockSpeed);

		// Rest
		SetUpBlocksInRow("data\\1HitBlock.png", BlockWidth, BlockHeight, 100, 100, 5, 1, BlockSpeed);
		SetUpBlocksInRow("data\\2HitBlock.png", BlockWidth, BlockHeight, 100, 60, 5, 2, BlockSpeed);
		SetUpBlocksInRow("data\\1HitBlock.png", BlockWidth, BlockHeight, 100, 20 , 5, 1, BlockSpeed);
		SetUpBlocksInRow("data\\1HitBlock.png", BlockWidth, BlockHeight, 100, -120, 5, 1, BlockSpeed);
		SetUpBlocksInRow("data\\2HitBlock.png", BlockWidth, BlockHeight, 100, -160, 5, 2, BlockSpeed);
		SetUpBlocksInRow("data\\1HitBlock.png", BlockWidth, BlockHeight, 100, -200, 5, 1, BlockSpeed);



	   /* Others initial settings */
		GameState = GameState::INITIAL;
		showCursor(true);
		VictoryScreen = createSprite("data\\Win.png");
		setSpriteSize(VictoryScreen, EndScreenWidth, EndScreenHeight);
		DefeatScreen = createSprite("data\\Lose.png");
		setSpriteSize(DefeatScreen, EndScreenWidth, EndScreenHeight);
		ThrowingPhaseStartTime = 0;

	   /* DEBUGGING */ 
		DebugPoint = createSprite("data\\Debug.png");
		setSpriteSize(DebugPoint, 10, 10);

		return true;
	}

	void Close() 
	{
	}

	/***********************************************************************
	*                                  TICK                                *
	************************************************************************/

	bool Tick() 
	{
		/* Checking Lose Condition */
		if (GameState == GameState::DEFEAT)
		{
			drawSprite(DefeatScreen, 154, 248);
			return false;
		}

		/* Checking Win Condition */
		if (GameState == GameState::VICTORY)
		{
			drawSprite(VictoryScreen, 154, 248);
			return false;
		}

		/* Losing player's lives logic */
		if (Ball.Coordinates.Y > MapHeight)
		{
			if (!Lives.empty()) 
			{
				Lives.pop_back();
			}
			if (Lives.empty()) 
			{
				GameState = GameState::DEFEAT;
				showCursor(true);
			}
			else
			{
				ResetBallPosition();
			}
			showCursor(true);
			return false;
		}

		/*   Finish Modifier's Effects  */
		for (int i = 0; i < Modifiers.size(); i++)
		{
			if (Modifiers[i]->TimeToStopEffect > 0 && getTickCount() - Modifiers[i]->TimeToStopEffect < 1000)
			{
				FinishExecuteModifierLogic(Modifiers[i]);
			}			
		}
		

		/*   Spawn Modifier   */
		if (getTickCount() - TimeBeforeSpawnNextMod < 1000)
		{
			SpawnNextMod();
			TimeBeforeSpawnNextMod = TimeBeforeSpawnNextMod + TimeSpawnIncrement;
			IsNextModPositive = !IsNextModPositive;
		}


		/***********************************************************************
		*                          TICK - MOVEMENT PHASE                       *
		************************************************************************/
		// Player
		Player.HorizontalMove(WallOffet);

		if (GameState == GameState::INITIAL)
		{
			// Ball
			Ball.HorizontalMove(WallOffet + (Player.Width / 2) - (Player.Width / 10));
		}
		else
		{
			// Ball
			Ball.FreeMove();
			
		}

		if (GameHasStart)
		{
			//Modifiers
			for (int i = 0; i < Modifiers.size(); i++)
			{
				if (!Modifiers[i]->IsPlayerCollisionDone)
					Modifiers[i]->Modifier->FreeMove();
			}

			// Blocks
			for (int i = 0; i < Blocks.size(); i++)
			{
				Blocks[i]->FreeMove();
			}

			/*  Check if Ball is reaching the precalculate collision points  */
			pair<bool, SCoordinates> BallReachIntersection = ObjectReachNextIntersection(Ball, NextBallCollisionPoints, BallErrorTolerance);
			if (BallReachIntersection.first)
			{
				ExecuteBallHitLogic(BallReachIntersection.second);
			}

			/*  Check if any Modifier is reaching the precalculate collision points  */
			for (int i = 0; i < Modifiers.size(); i++)
			{
				pair<bool, SCoordinates> ModifierReachIntersection = ObjectReachNextIntersection(*Modifiers[i]->Modifier, Modifiers[i]->CollisionPoints, BallErrorTolerance);
				if (ModifierReachIntersection.first)
				{
					ExecuteModifierLogic(Modifiers[i], ModifierReachIntersection.second);
				}
			}
		}

		/***********************************************************************
		*                          TICK - COLLISION PHASE                      *
		************************************************************************/

		/*   Avoiding collisions in the first milliseconds after player throw the ball */
		if (GameState == GameState::THROWING)
		{
			if (ThrowingPhaseStartTime <= 0)
			{
				ThrowingPhaseStartTime = getTickCount();
			}
			//After one second in the Throwing Phase we move to the next.
			if (getTickCount() - ThrowingPhaseStartTime > ThrowingPhaseDuration)
			{
				GameState = GameState::PLAYING;
			}

		}
		else   
		if (GameState == GameState::PLAYING)
		{
			NextBallCollisionPoints = CheckingBallCollisions();		
		}
		CheckingBlocksVsPlayerCollision();
		CheckingModifiersCollisions();


		/***********************************************************************
		*                          TICK - DRAWING PHASE                        *
		************************************************************************/
		drawTestBackground();
		DrawingObjects();



		/*  Workaround: the ball get trapped inside the player. If you hit the ball in its opposite direction, the collision system doesn't
		    have enough time to detect the player */
		if (GameState == GameState::PLAYING && Player.Coordinates.X <= Ball.Coordinates.X && Ball.Coordinates.X <= Player.Coordinates.X + Player.Width &&
			Player.Coordinates.Y <= Ball.Coordinates.Y && Ball.Coordinates.Y <= Player.Coordinates.Y + Player.Height)
		{
			cout << "\nThe ball trapped inside the player. Ocurrency number " << TrappedCount;
			ResetBallPosition(); // SetDirectionUnitVector doesn't work in this scenario, the ball is moving so fast
		}


	    return false;
	}

	/***********************************************************************
	*                            ON MOUSE MOVE                             *
	************************************************************************/

	void onMouseMove(int x, int y, int xrelative, int yrelative) 
	{
		if (GameState == GameState::INITIAL) 
		{
			FirstPositionAfterClick = SCoordinates((float)x, (float)y);
		}

	}

	/***********************************************************************
	*                        ON MOUSE BUTTON CLICK                         *
	************************************************************************/

	void onMouseButtonClick(FRMouseButton button, bool isReleased)
	{
		if (button == FRMouseButton::LEFT && GameState == GameState::INITIAL)
		{
			ThrowBall();
			GameHasStart = true;
		}

		if (button == FRMouseButton::RIGHT)
		{
			if (GameState == GameState::DEFEAT || GameState == GameState::VICTORY)
			{
				Init();
				GameHasStart = false;
			}
			else
				ResetBallPosition();
		}
	}

	/***********************************************************************
	*                            ON KEY PRESSED                            *
	************************************************************************/

	void onKeyPressed(FRKey key) 
	{
		if (key == FRKey::LEFT)
		{
			Player.bIsMovingRight = false;
			Player.bIsMovingLeft = true;
			if (GameState == GameState::INITIAL) 
			{
				Ball.bIsMovingRight = false;
				Ball.bIsMovingLeft = true;
			}
			
		}
		else
		if (key == FRKey::RIGHT) 
		{
			Player.bIsMovingLeft = false;
			Player.bIsMovingRight = true;
			if (GameState == GameState::INITIAL)
			{
				Ball.bIsMovingLeft = false;
				Ball.bIsMovingRight = true;
			}
			
		}

	}

	/***********************************************************************
	*                            ON KEY RELEASED                           *
	************************************************************************/

	void onKeyReleased(FRKey key) 
	{
		if (key == FRKey::LEFT)
		{
			Player.bIsMovingLeft = false;
			if (GameState == GameState::INITIAL)
			{
				Ball.bIsMovingLeft = false;
			}
		}
		else
		if (key == FRKey::RIGHT)
		{
			Player.bIsMovingRight = false;
			if (GameState == GameState::INITIAL)
			{
				Ball.bIsMovingRight = false;
			}
		}

	}
	
	const char* GetTitle() override
	{
		return "Arcanoid";
	}
	};


	/***********************************************************************
	*                                MAIN                                  *
	************************************************************************/

	int main(int argc, char *argv[])
	{
		return run(new Arkanoid);
	}
