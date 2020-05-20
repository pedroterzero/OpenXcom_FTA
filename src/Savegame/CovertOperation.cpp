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
#include "CovertOperation.h"
#include <assert.h>
#include "../Engine/Language.h"
#include "../Engine/Game.h"
#include "../fmath.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"
#include "../Savegame/SavedGame.h"
#include "../Geoscape/GeoscapeEventState.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Transfer.h"
#include "../Savegame/AlienStrategy.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleSoldierBonus.h"
#include "../Mod/RuleAlienMission.h"




namespace OpenXcom
{
CovertOperation::CovertOperation(const RuleCovertOperation* rule, Base* base, int cost) :
	_rule(rule), _base(base), _dailyProgress(0), _spent(0), _cost(cost), _successChance(0), _assignedScientists(0), _assignedEngineers(0), _inBattlescape(false), _interrupted(false)
{
	_items = new ItemContainer();
	if (base != 0)
	{
		setBase(base);
	}
}

CovertOperation::~CovertOperation()
{
	delete _items;
}

/**
* Loads the event from YAML.
* @param node The YAML node containing the data.
*/
void CovertOperation::load(const YAML::Node& node)
{
	_dailyProgress = node["dailyProgress"].as<int>(_dailyProgress);
	_spent = node["spent"].as<int>(_spent);
	_cost = node["cost"].as<int>(_cost);
	_successChance = node["successChance"].as<int>(_successChance);
	_assignedScientists = node["assignedScientists"].as<int>(_assignedScientists);
	_assignedEngineers = node["assignedEngineers"].as<int>(_assignedEngineers);
	_inBattlescape =  node["inBattlescape"].as<bool>(_inBattlescape);

	_items->load(node["items"]);
}

/**
	* Saves the event to YAML.
	* @return YAML node.
	*/
YAML::Node CovertOperation::save() const
{
	YAML::Node node;
	node["name"] = getRules()->getName();
	node["dailyProgress"] = _dailyProgress;
	node["spent"] = _spent;
	node["cost"] = _cost;
	node["successChance"] = _successChance;
	node["assignedScientists"] = _assignedScientists;
	node["assignedEngineers"] = _assignedEngineers;
	node["inBattlescape"] = _inBattlescape;

	node["items"] = _items->save();

	return node;
}


/**
 * Loads a Covert Operation name from rules.
 * @return operation name.
 */
std::string CovertOperation::getOperationName()
{
	return _rule->getName();
}

/**
 * Return a string describing CovertOperation success odds.
 * @par chance is input to calculate chance name.
 * @par mode to define if we need to calculate string or get existing mission chance.
 * @return a string describing CovertOperation success odds.
 */
std::string CovertOperation::getOddsName(int chance, bool mode) const
{
	int operationChance;
	if (!mode)
	{
		operationChance = _successChance;
	}
	else
	{
		operationChance = chance;
	}
	if (operationChance > 100)
	{
		return ("STR_GREAT");
	}
	else if (operationChance > 75)
	{
		return ("STR_GOOD");
	}
	else if (operationChance > 50)
	{
		return ("STR_AVARAGE");
	}
	else if (operationChance > 25)
	{
		return ("STR_POOR");
	}
	else if (operationChance > 0)
	{
		return ("STR_VERY_LOW");
	}
	else
	{
		return ("STR_NONE");
	}
}

/**
 * Return a string describing approximate time before operation results.
 * @return a string describing time left.
 */
std::string CovertOperation::getTimeLeftName() const
{
	int timeLift = (_cost - _spent) / _dailyProgress;
	if (timeLift > 45)
	{
		return ("STR_SEVERAL_MONTHS");
	}
	else if (timeLift > 20)
	{
		return ("STR_MONTH");
	}
	else if (timeLift > 10)
	{
		return ("STR_SEVERAL_WEEKS");
	}
	else if (timeLift > 5)
	{
		return ("STR_WEEK");
	}
	else
	{
		return ("STR_SEVERAL_DAYS");
	}
}

/**
* Handle Faction logic.
* @param Game game engine.
* @param ThinkPeriond - timestep to determine think process
*/
//bool CovertOperation::think(Game& engine, ThinkPeriond period)
//{
//	const Mod& mod = *engine.getMod();
//	SavedGame& game = *engine.getSavedGame();
//	//lets process out daily duty
//	if (period == TIMESTEP_DAILY)
//	{
//		//check if we discover our faction
//		if (game.isResearched(mod.getResearch(_rule.getDiscoverResearch())) && !_discovered)
//		{
//			setDiscovered(true);
//			//spawn celebration event if faction wants it
//			if (!_rule.getDiscoverEvent().empty())
//			{
//				RuleEvent* eventRules = mod.getEvent(_rule.getDiscoverEvent());
//				GeoscapeEvent* newEvent = new GeoscapeEvent(*eventRules);
//				int minutes = (eventRules->getTimer() + (RNG::generate(0, eventRules->getTimerRandom()))) / 30 * 30;
//				if (minutes < 30) minutes = 30; // just in case
//				newEvent->setSpawnCountdown(minutes);
//				game.getGeoscapeEvents().push_back(newEvent);
//				// remember that it has been generated
//				game.addGeneratedEvent(eventRules);
//			}
//			// update reputation level for just discovered fraction
//			updateReputationLevel();
//			// and if it turns friendly at start we sign help treaty by default
//			if (_reputationLvL > 0)
//			{
//				_treaties.push_back("STR_HELP_TREATY");
//			}
//		}
//		// let's see 
//		if (std::find(_treaties.begin(), _treaties.end(), "STR_HELP_TREATY") != _treaties.end())
//		{
//			bool generate = factionMissionGenerator(engine);
//			return generate;
//		}
//	}
//	return false;
//}
//bool CovertOperation::factionMissionGenerator(Game& engine)
//{
//	const Mod& mod = *engine.getMod();
//	SavedGame& game = *engine.getSavedGame();
//	if (RNG::percent(_rule.getGenMissionFrequency()))
//	{
//		auto scriptType = _rule.chooseGenMissionScriptType();
//		if (!scriptType.empty())
//		{
//			const RuleMissionScript& ruleScript = *mod.getMissionScript(scriptType);
//			auto month = game.getMonthsPassed();
//			// lets process mission script with own way, first things first
//			if (ruleScript.getFirstMonth() <= month &&
//				(ruleScript.getLastMonth() >= month || ruleScript.getLastMonth() == -1) &&
//				// make sure we haven't hit our run limit, if we have one
//				(ruleScript.getMaxRuns() == -1 || ruleScript.getMaxRuns() > game.getAlienStrategy().getMissionsRun(ruleScript.getVarName())) &&
//				// and make sure we satisfy the difficulty restrictions
//				(month < 1 || ruleScript.getMinScore() <= game.getCurrentScore(month)) &&
//				(month < 1 || ruleScript.getMaxScore() >= game.getCurrentScore(month)) &&
//				(month < 1 || ruleScript.getMinFunds() <= game.getFunds()) &&
//				(month < 1 || ruleScript.getMaxFunds() >= game.getFunds()) &&
//				ruleScript.getMinDifficulty() <= game.getDifficulty())
//			{
//				// level two condition check: make sure we meet any research requirements, if any.
//				bool triggerHappy = true;
//				for (std::map<std::string, bool>::const_iterator j = ruleScript.getResearchTriggers().begin(); triggerHappy && j != ruleScript.getResearchTriggers().end(); ++j)
//				{
//					triggerHappy = (game.isResearched(j->first) == j->second);
//					if (!triggerHappy)
//						return false;
//				}
//				if (triggerHappy)
//				{
//					// item requirements
//					for (auto& triggerItem : ruleScript.getItemTriggers())
//					{
//						triggerHappy = (game.isItemObtained(triggerItem.first) == triggerItem.second);
//						if (!triggerHappy)
//							return false;
//					}
//				}
//				if (triggerHappy)
//				{
//					// facility requirements
//					for (auto& triggerFacility : ruleScript.getFacilityTriggers())
//					{
//						triggerHappy = (game.isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
//						if (!triggerHappy)
//							return false;
//					}
//				}
//				// levels one and two passed: insert this command into the array.
//				if (triggerHappy)
//				{
//					_generatedCommandType = ruleScript.getType();
//					return true;
//				}
//			}
//		}
//	}
//	return false;
//}



}
