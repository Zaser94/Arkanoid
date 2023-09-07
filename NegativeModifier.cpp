#include "NegativeModifier.h"
#include <random>

ANegativeModifier::ANegativeModifier(int width, int height, float x, float y, float speed, ENegativeModifier negativeMod)
	:AMovableObject(width, height, x, y, speed)
{
	NegativeMod = negativeMod;

	switch (NegativeMod)
	{
	case ENegativeModifier::DAMAGE: Path = "data\\Damage.png";
		break;
	case ENegativeModifier::SLOWPLAYER: Path = "data\\SlowPlayer.png";
		break;
	case ENegativeModifier::FASTBALL:Path = "data\\FastBall.png";
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