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

namespace OpenXcom
{
/**
* Define what time frame should think logic process.
*/
enum ThinkPeriond { TIMESTEP_DAILY, TIMESTEP_MONTHLY };

typedef std::pair<std::string, int> CovertOperationId;

class RuleCovertOperation;
class Game;
class SavedGame;
class Mod;
class GeoscapeEvent;
class RuleEvent;
class RuleAlienMission;
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
	int _dailyProgress, _spent, _cost;
	int _successChance, _assignedScientists, _assignedEngineers;
	ItemContainer* _items;
	bool _inBattlescape, _interrupted;
	std::string _generatedMission;
public:
	/// Creates a blank Covert Operation.
	CovertOperation(const RuleCovertOperation* rule, Base* base, int cost = 0);
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
	void setBase(Base* base) { _base = base; };
	/// Gets operation name
	std::string getOperationName();
	/// Gets daily progress for this operation.
	int getDailyProgress() const { return _dailyProgress; };
	/// Sets daily progress for this operation.
	void setDailyProgress(int dailyProgress) { _dailyProgress = dailyProgress; };
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

	/// Gets the craft's items.
	ItemContainer* getItems() { return _items; };

	/// Gets approximate odds of operation success for UI.
	std::string getOddsName(int chance = 0, bool mode = false) const;
	/// Get approximate time before operation results.
	std::string getTimeLeftName() const;





	/*/// Gets corrent reputation level.
	int getReputationLevel();
	/// Gets corrent reputation level name.
	std::string getReputationName();
	/// Update reputation level based on current reputation of the faction.
	void updateReputationLevel();
	/// Is this Faction was discovered?
	bool isDiscovered() const { return _discovered; }
	/// Sets Faction's discovered status.
	void setDiscovered(bool status);
	/// Handle Faction logic.
	bool think(Game& engine, ThinkPeriond = TIMESTEP_DAILY);
	/// Generates mission for the faction base on current situation in the game.
	bool factionMissionGenerator(Game& engine);
	/// Get mission script command that pass all checks to generate alien mission for that faction.
	std::string getCommandType() const { return _generatedCommandType; };*/
};
}
