#include "Pickup.h"
#include "Capital.h"

Pickup::Pickup(Capital::ECapitalType type, Vector2 startPos)
{
	using namespace Capital;

	// Setup style
	switch (type)
	{
	case ECapitalType::Tree: size = { 8, 4 }; color = RED; break;
	case ECapitalType::Coal: size = { 2, 2 }; color = BLACK; break;
	case ECapitalType::IronOre: size = { 4, 4 }; color = GRAY; break;
	case ECapitalType::IronBar: size = { 4, 2 }; color = GRAY; break;
	case ECapitalType::Sword: size = { 3, 1 }; color = WHITE; break;
	}

	this->type = type;
	position = startPos;
}

void Pickup::Render()
{
	DrawRectangle(position.x, position.y, size.x, size.y, color);
}