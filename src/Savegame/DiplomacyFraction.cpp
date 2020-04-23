/*
 * Copyright 2010-2019 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "DiplomacyFraction.h"
#include <assert.h>
#include "../Mod/RuleDiplomacyFraction.h"

namespace OpenXcom
{

DiplomacyFraction::DiplomacyFraction(const RuleDiplomacyFraction &rule) : _rule(rule), _reputation(0), _discovered(false)
{
	// Empty by design.
}

DiplomacyFraction::~DiplomacyFraction()
{
	// Empty by design.
}

/**
 * Loads the event from YAML.
 * @param node The YAML node containing the data.
 */
void DiplomacyFraction::load(const YAML::Node &node)
{
	_reputation = node["reputation"].as<int>(_reputation);
	_discovered = node["discovered"].as<bool>(_discovered);
}

/**
 * Saves the event to YAML.
 * @return YAML node.
 */ 
YAML::Node DiplomacyFraction::save() const
{
	YAML::Node node;
	node["name"] = _rule.getName();
	node["reputation"] = _reputation;
	if (_discovered)
	{
		node["discovered"] = _discovered;
	}
	return node;
}

/**
 * Sets current player's reputation in this fraction.
 * @param new reputation value.
 */
void DiplomacyFraction::setReputation(int reputation)
{
	_reputation = reputation;
}

int DiplomacyFraction::getReputationLevel() const
{
	if (_reputation >= 75)
	{
		return 3;
	}
	else if (_reputation >= 50 && _reputation < 75)
	{
		return 2;
	}
	else if (_reputation >= 25 && _reputation < 50)
	{
		return 1;
	}
	else if (_reputation >= -25 && _reputation < 25)
	{
		return 0;
	}
	else if (_reputation <= -50 && _reputation < 25)
	{
		return -1;
	}
	else if (_reputation <= -50 && _reputation < 25)
	{
		return -2;
	}
	else if (_reputation <= -50 && _reputation < 25)
	{
		return -3;
	}
	else { return 0; }
}

std::string DiplomacyFraction::getReputationName() const
{
	int repLvl = getReputationLevel();
	if (repLvl == 3)
	{
		return "STR_ALLY";
	}
	else if (repLvl == 2)
	{
		return "STR_HONORED";
	}
	else if (repLvl == 1)
	{
		return "STR_FRIENDLY";
	}
	else if (repLvl == 0)
	{
		return "STR_NEUTRAL";
	}
	else if (repLvl == -1)
	{
		return "STR_UNFRIENDLY";
	}
	else if (repLvl == -2)
	{
		return "STR_HOSTILE";
	}
	else if (repLvl == -3)
	{
		return "STR_HATED";
	}
	else
	{
		return "STR_ERROR";
	}

}

/**
 * Sets status of the fraction to the player, was it discovered or not.
 * @param isDiscovered status.
 */
void DiplomacyFraction::setDiscovered(bool status)
{
	_discovered = status;
}

//void DiplomacyFraction::think()
//{
//	// if finished, don't do anything
//	if (_over)
//	{
//		return;
//	}
//
//	// are we there yet?
//	if (_spawnCountdown > 30)
//	{
//		_spawnCountdown -= 30;
//		return;
//	}
//
//	// ok, the time has come...
//	_over = true;
//}

}
