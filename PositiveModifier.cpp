#include "PositiveModifier.h"
#include <random>

APositiveModifier::APositiveModifier(int width, int height, float x, float y, float speed, EPositiveModifier positiveMod)
	:AMovableObject(width,  height,  x,  y,  speed)
{
	PositiveMod = positiveMod;

	switch (PositiveMod)
	{
	case EPositiveModifier::EXTRAWALL: Path = "data\\ExtraWall.png";
		break;
	case EPositiveModifier::MAGNET: Path = "data\\Magnet.png";
		break;
	case EPositiveModifier::LONGPLAYER: Path = "data\\LongPlayer.png";
		break;
	default:
		break;
	}

	// Sets a random direction
	std::random_device rd; // obtain a random number from hardware
	std::mt19937 gen(rd()); // seed the generator
	std::uniform_int_distribution<> distr(-100, 100); // define the range

	SetDirectionUnitVector(distr(gen), distr(gen), distr(gen), distr(gen));
    
}