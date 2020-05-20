/*
 * Copyright 2010-2016 OpenXcom Developers.
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
#include <algorithm>
#include "RuleCovertOperation.h"
#include "../Engine/RNG.h"
#include "../fmath.h"
#include "../Engine/Exception.h"
#include "Mod.h"

namespace OpenXcom
{

/**
* Creates a blank ruleset for a certain
* type of RuleCovertOperation.
* @param type String defining the type.
*/
RuleCovertOperation::RuleCovertOperation(const std::string& name) : _name(name), _description("NONE"), _covertSoldierJoinFight(false), _soldierSlots(1), _optionalSoldierSlots(0),
																	_scientistSlots(0), _engineerSlots(0), _optionalSoldierEffect(15), _scientistEffect(10), _engeneerEffect(10),
																	_baseChances(50), _costs(0), _itemSpaceLimit(-1), _itemSpaceEffect(10), _successScore(0), _failureScore(0), _progressEventChance(0), _armorEffect(20),
																	_listOrder(0)
{
}

RuleCovertOperation::~RuleCovertOperation()
{
}

/**
* Loads the craft from a YAML file.
* @param node YAML node.
* @param mod Mod for the CovertOperation.
* @param modIndex A value that offsets the sounds and sprite values to avoid conflicts.
* @param listOrder The list weight for this CovertOperation.
*/
void RuleCovertOperation::load(const YAML::Node& node, Mod* mod, int listOrder)
{
	if (const YAML::Node& parent = node["refNode"])
	{
		load(parent, mod, listOrder);
	}
	_name = node["name"].as<std::string>(_name);
	_description = node["description"].as<std::string>(_description);
	_successEvent = node["successEvent"].as<std::string>(_successEvent);
	_failureEvent = node["failureEvent"].as<std::string>(_failureEvent);
	_progressEvent = node["progressEvent"].as<std::string>(_progressEvent);
	_requires = node["requires"].as<std::vector<std::string>>(_requires);
	mod->loadBaseFunction(_name, _requiresBuyBaseFunc, node["requiresBuyBaseFunc"]);
	_soldierSlots = node["soldierSlots"].as<int>(_soldierSlots);
	if (_soldierSlots < 1)
	{
		throw Exception("Error in loading operation '" + _name + "'! It has less than 1 soldier.");
	}
	_optionalSoldierSlots = node["optionalSoldierSlots"].as<int>(_optionalSoldierSlots);
	_scientistSlots = node["scientistSlots"].as<int>(_scientistSlots);
	_engineerSlots = node["engineerSlots"].as<int>(_engineerSlots);
	_optionalSoldierEffect = node["optionalSoldierEffect"].as<int>(_optionalSoldierEffect);
	_scientistEffect = node["scientistEffect"].as<int>(_scientistEffect);
	_engeneerEffect = node["engeneerEffect"].as<int>(_engeneerEffect);
	_covertSoldierJoinFight = node["covertSoldierJoinFight"].as<bool>(_covertSoldierJoinFight);
	_baseChances = node["baseChances"].as<int>(_baseChances);
	_costs = node["costs"].as<int>(_costs);
	_successScore = node["successScore"].as<int>(_successScore);
	_failureScore = node["failureScore"].as<int>(_failureScore);
	_progressEventChance = node["_progressEventChance"].as<int>(_progressEventChance);
	if (node["successMissions"])
	{
		_successMissions.load(node["successMissions"]);
	}
	if (node["failureMissions"])
	{
		_failureMissions.load(node["failureMissions"]);
	}
	_requiredReputationLvl = node["requiredReputationLvl"].as<std::map<std::string, int>>(_requiredReputationLvl);
	_successReputationScore = node["successReputationScore"].as<std::map<std::string, int>>(_successReputationScore);
	_failureReputationScore = node["failureReputationScore"].as<std::map<std::string, int>>(_failureReputationScore);
	_itemSpaceLimit = node["itemSpaceLimit"].as<double>(_itemSpaceLimit);
	_itemSpaceEffect = node["itemSpaceEffect"].as<double>(_itemSpaceEffect);
	_requiredItems = node["requiredItems"].as<std::map<std::string, int>>(_requiredItems);
	_allowedArmor = node["allowedArmor"].as<std::vector<std::string>>(_allowedArmor);
	_armorEffect = node["armorEffect"].as<int>(_armorEffect);
	_soldierTypeEffectiveness = node["soldierTypeEffectiveness"].as<std::map<std::string, int>>(_soldierTypeEffectiveness);
	if (!_listOrder)
	{
		_listOrder = listOrder;
	}
}

std::string RuleCovertOperation::chooseGenSuccessMissionType() const
{
	return _successMissions.choose();
}

std::string RuleCovertOperation::chooseGenFailureMissionType() const
{
	return _failureMissions.choose();
}

}

