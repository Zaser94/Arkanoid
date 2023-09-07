#pragma once
#include "MovableObject.h"

enum class EPositiveModifier {
	EXTRAWALL,
	MAGNET,
	LONGPLAYER,
	COUNT
};

class APositiveModifier : public AMovableObject
{
private:

	EPositiveModifier PositiveMod;
	const char* Path = 0;

public:
	APositiveModifier() {};
	APositiveModifier(int width, int height, float x, float y, float speed, EPositiveModifier positiveMod);
	const char* GetPath() { return Path; };
	EPositiveModifier GetType() { return PositiveMod; }

};