#pragma once
/*
 * Copyright 2010-2020 OpenXcom Developers.
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
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>
#include "../Savegame/WeightedOptions.h"
#include "RuleBaseFacilityFunctions.h"

namespace OpenXcom
{

class Mod;

/**
 * Represents ruleset for a specific covert operation.
 * Contains constant info about a covert operation like
 * name, requirments, etc.
 * @sa CovertOperation
 */
class RuleCovertOperation
{
private:
	std::string _name, _description, _successEvent, _failureEvent, _progressEvent;
	std::vector<std::string> _requires, _canceledBy, _allowedArmor;
	RuleBaseFacilityFunctions _requiresBuyBaseFunc;
	int _soldierSlots, _optionalSoldierSlots, _scientistSlots, _engineerSlots, _optionalSoldierEffect, _scientistEffect, _engeneerEffect, _itemSpaceEffect, _armorEffect;
	double _itemSpaceLimit;
	int _baseChances, _costs, _successScore, _failureScore, _progressEventChance;
	bool _covertSoldierJoinFight;
	WeightedOptions _successMissions, _failureMissions;
	std::map<std::string, int> _requiredReputationLvl, _successReputationScore, _failureReputationScore, _requiredItems, _soldierTypeEffectiveness;
	int _listOrder;
public:
	/// Creates a blank craft ruleset.
	RuleCovertOperation(const std::string& name);
	/// Cleans up the craft ruleset.
	~RuleCovertOperation();
	/// Loads craft data from YAML.
	void load(const YAML::Node& node, Mod* mod, int listOrder);
	/// Gets the operation's name.
	const std::string& getName() const { return _name; };
	/// Gets the operation's description.
	const std::string& getDescription() const { return _description; };
	/// Gets the operation's requirements.
	const std::vector<std::string>& getRequirements() const { return _requires; };
	/// Gets the base functions required to start operation.
	RuleBaseFacilityFunctions getRequiresBuyBaseFunc() const { return _requiresBuyBaseFunc; }
	/// Gets the research name that would make this operation impossible to run.
	const std::vector<std::string>& getCanceledBy() const { return _canceledBy; };
	/// Gets the event name that would be spawned on success operation result.
	const std::string& getSuccessEvent() const { return _successEvent; };
	/// Gets the event name that would be spawned on failure operation result.
	const std::string& getFailureEvent() const { return _failureEvent; };
	/// Gets the event name that would be spawned while operation is in progress.
	const std::string& getProgressEvent() const { return _progressEvent; };
	/// Gets the soldiers number that required by this operations.
	int getSoldierSlots() const { return _soldierSlots; };
	/// Gets the optional soldiers slots for this operations.
	int getOptionalSoldierSlots() const { return _optionalSoldierSlots; };
	/// Gets the optional scientist slots for this operations.
	int getScientistSlots() const { return _scientistSlots; };
	/// Gets the optional engeneer slots for this operations.
	int getEngineerSlots() const { return _engineerSlots; };
	/// Gets if covert soldiers would join the battlescape
	bool getIfCovertSoldierJoinFight() const { return _covertSoldierJoinFight; }
	/// Gets the optional soldiers slots effectiveness for this operations.
	int getOptionalSoldierEffect() const { return _optionalSoldierEffect; };
	/// Gets the optional scientist slots effectiveness for this operations.
	int getScientistEffect() const { return _scientistEffect; };
	/// Gets the optional engeneer slots effectiveness for this operations.
	int getEngeneerEffect() const { return _engeneerEffect; };
	/// Gets the operation's base chances of success result.
	int getBaseChances() const { return _baseChances; };
	/// Gets the operation's time costs.
	int getCosts() const { return _costs; };
	/// Gets the operation's global score, that is awarded on success result.
	int getSuccessScore() const { return _successScore; };
	/// Gets the operation's global score, that is awarded on failure.
	int getFailureScore() const { return _failureScore; };
	/// Gets chance of spawning event in progress of running covert operation.
	int getProgressEventChance() const { return _progressEventChance; };
	/// Gets operation item space limit.
	double getItemSpaceLimit() const { return _itemSpaceLimit; }
	/// Gets operation item space limit.
	int getItemSpaceEffect() const { return _itemSpaceEffect; }
	/// Gets chosen alien mission to run as success resolve of covert operation.
	std::string chooseGenSuccessMissionType() const;
	/// Gets chosen alien mission to run as failure resolve of covert operation.
	std::string chooseGenFailureMissionType() const;
	/// Gets the faction's required reputation list for this operation.
	const std::map<std::string, int>& getRequiredReputationLvlList() const { return _requiredReputationLvl; }
	/// Gets the factions reputation award list for this operation on success result.
	const std::map<std::string, int>& getSuccessReputationScoreList() const { return _successReputationScore; }
	/// Gets the factions reputation award list for this operation on failure.
	const std::map<std::string, int>& getFailureReputationScoreList() const { return _failureReputationScore; }
	/// Gets the operation's required item list.
	const std::map<std::string, int>& getRequiredItemList() const { return _requiredItems; }
	/// Gets the operation's allowed armor.
	const std::vector<std::string>& getAllowedArmor() const { return _allowedArmor; };
	/// Gets the operation's allowed armor effect.
	int getAllowedArmorEffect() const { return _armorEffect; };
	/// Gets the effectivness by soldier type for this operations.
	const std::map<std::string, int>& getSoldierTypeEffectiveness() const { return _soldierTypeEffectiveness; }
	/// Gets the operation's list order.
	int getListOrder() const { return _listOrder; }
};
}
