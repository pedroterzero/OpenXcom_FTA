#pragma once
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
#include <string>
#include <yaml-cpp/yaml.h>
//#include "../Engine/State.h"

namespace OpenXcom
{

class RuleCovertOperation;
class Game;
class SavedGame;
class Globe;
class Region;
class Mod;
class GeoscapeEvent;
class RuleEvent;
class RuleAlienMission;
class DiplomacyFaction;
class Soldier;
class Base;
class ItemContainer;
class RuleStartingCondition;

/**
* Represents a Covert Operation.
*/
class CovertOperation
{
private:
	const RuleCovertOperation* _rule;
	Base* _base;
	int _spent, _cost;
	int _successChance, _assignedScientists, _assignedEngineers;
	ItemContainer* _items;
	bool _inBattlescape, _hasBattlescapeResolve, _over, _hasPsi, _progressEventSpawned;
	std::string _generatedMission;
public:
	/// Creates a blank Covert Operation.
	CovertOperation(const RuleCovertOperation* rule, Base* base, int cost = 0, int chances = 0);
	/// Cleans up the Covert Operation info.
	~CovertOperation();
	/// Loads the Covert Operation from YAML.
	void load(const YAML::Node& node);
	/// Saves the Covert Operation to YAML.
	YAML::Node save() const;
	/// Gets the operation's ruleset.
	const RuleCovertOperation* getRules() const { return _rule; };
	/// Gets the operation's base.
	Base* getBase() const { return _base; };
	/// Sets the operation's base.
	void setBase(Base* base);
	/// Gets operation name
	std::string getOperationName();
	/// Gets spent effort for this operation.
	int getSpent() const { return _spent; };
	/// Sets spent effort for this operation.
	void setSpent(int spent) { _spent = spent; };
	/// Gets cost for this operation.
	int getCost() const { return _cost; };
	/// Sets cost for this operation.
	void setCost(int cost) { _cost = cost; };

	/// Gets success chance for this operation.
	int getSuccessChance() const { return _successChance; };
	/// Sets success chance for this operation.
	void setSuccessChance(int successChance) { _successChance = successChance; };
	/// Gets assigned scientists for this operation.
	int getAssignedScientists() const { return _assignedScientists; };
	/// Sets assigned scientists for this operation.
	void setAssignedScientists(int assignedScientists) { _assignedScientists = assignedScientists; };
	/// Gets assigned Engineers for this operation.
	int getAssignedEngineers() const { return _assignedEngineers; };
	/// Sets assigned Engineers for this operation.
	void setAssignedEngineers(int assignedEngineers) { _assignedEngineers = assignedEngineers; };
	/// Gets if operation has psionic agents capable to use their abilities.
	int getIsPsi() const { return _hasPsi; };
	/// Sets if operation has psionic agents capable to use their abilities.
	void setIsPsi(int hasPsi) { _hasPsi = hasPsi; };

	/// Gets operation's items.
	ItemContainer* getItems() { return _items; };

	/// Gets approximate odds of operation success for UI.
	std::string getOddsName();
	/// Get approximate time before operation results.
	std::string getTimeLeftName();

	/// Is this covert operation over?
	bool isOver() const { return _over; }
	/// Is this covert operation is running in battlescape?
	bool isInBattlescape() const { return _inBattlescape; }
	/// Sets if this operation is running in battlescape
	void setInBattlescape(bool inbattle) { _inBattlescape = inbattle; }
	/// Handle covert operation logic.
	void think(Game& engine, const Globe& globe);
	/// Generates event for covert operation.
	void spawnEvent(Game& engine, std::string eventName);
	/// Process soldier's actions results in covert operation
	void backgroundSimulation(Game& engine, bool operationResult, bool criticalFail, int woundOdds, int deathOdds);
	/// Converts the number of experience to the stat increase.
	int improveStat(int exp) const;
	/// Takes care to finish covert operation.
	void finishOperation();
};
}
