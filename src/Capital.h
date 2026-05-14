#pragma once
#include <stddef.h>
#include "types.h"

/*
Capital represents exchangeable "gameplay" resources like trees and iron
Capital types and operations are defined here
*/

namespace Capital
{
	// Tracks amounts of the resources for each type
	// Can be used for stored or defining how resources cost - e.g. how much wood and iron will cost to build a smelter
	struct CapitalAmounts
	{
		int amounts[ECapitalType::ECapitalTypeCount];

		void Empty()
		{
			amounts[ECapitalType::Tree] = 0;
			amounts[ECapitalType::Coal] = 0;
			amounts[ECapitalType::IronOre] = 0;
			amounts[ECapitalType::IronBar] = 0;
			amounts[ECapitalType::Sword] = 0;
		}

		CapitalAmounts(int trees = 0, int coal = 0, int ironOres = 0, int ironBars = 0, int swords = 0)
		{
			amounts[ECapitalType::Tree] = trees;
			amounts[ECapitalType::Coal] = coal;
			amounts[ECapitalType::IronOre] = ironOres;
			amounts[ECapitalType::IronBar] = ironBars;
			amounts[ECapitalType::Sword] = swords;
		}

		int operator[](size_t idx)
		{
			return amounts[idx];
		}

		inline const int Size() { return ECapitalType::ECapitalTypeCount; }

		// Assign
		inline CapitalAmounts& operator=(const CapitalAmounts& rhs)
		{
			for (size_t i = 0; i < ECapitalType::ECapitalTypeCount; i++)
			{
				amounts[i] = rhs.amounts[i];
			}

			return *this;
		}

		inline CapitalAmounts& operator+=(const CapitalAmounts& rhs)
		{
			for (size_t i = 0; i < ECapitalType::ECapitalTypeCount; i++)
			{
				amounts[i] += rhs.amounts[i];
			}

			return *this;
		}

		inline CapitalAmounts& operator-=(const CapitalAmounts& rhs)
		{
			for (size_t i = 0; i < ECapitalType::ECapitalTypeCount; i++)
			{
				amounts[i] -= rhs.amounts[i];
			}

			return *this;
		}

		// Comparison
		inline bool operator>=(const CapitalAmounts& rhs)
		{
			for (size_t i = 0; i < ECapitalType::ECapitalTypeCount; i++)
			{
				if (amounts[i] < rhs.amounts[i])
					return false;
			}

			return true;
		}
	};

	struct ActionCost
	{
		float time;
		CapitalAmounts capital;
	};
}

