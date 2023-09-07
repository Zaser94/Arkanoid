#pragma once
#include "MovableObject.h"

enum class ENegativeModifier {
	DAMAGE,
	FASTBALL,
	SLOWPLAYER,
	COUNT
};

class ANegativeModifier : public AMovableObject
{
private:

	ENegativeModifier NegativeMod;
	const char* Path;

public:
	ANegativeModifier() {};
	ANegativeModifier(int width, int height, float x, float y, float speed, ENegativeModifier positiveMod);
	const char* GetPath() { return Path; };
	ENegativeModifier GetType() { return NegativeMod; }

};