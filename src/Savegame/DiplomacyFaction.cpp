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
#include "DiplomacyFaction.h"
#include <assert.h>
#include "../Engine/Game.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"
#include "../Geoscape/GeoscapeEventState.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleEventScript.h"
#include "../Mod/RuleMissionScript.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Savegame/AlienStrategy.h"
#include "../fmath.h"
#include "../FTA/MasterMind.h"

namespace OpenXcom
{

DiplomacyFaction::DiplomacyFaction(const RuleDiplomacyFaction &rule) : _rule(rule), _reputationScore(0), _reputationLvL(0), _discovered(false), _thisMonthDiscovered(false)
{
	// Empty by design.
}

DiplomacyFaction::~DiplomacyFaction()
{
	// Empty by design.
}

/**
 * Loads the event from YAML.
 * @param node The YAML node containing the data.
 */
void DiplomacyFaction::load(const YAML::Node &node)
{
	_reputationScore = node["reputationScore"].as<int>(_reputationScore);
	_reputationLvL = node["reputationLvL"].as<int>(_reputationLvL);
	_reputationName = node["reputationName"].as<std::string>(_reputationName);
	_discovered = node["discovered"].as<bool>(_discovered);
	_thisMonthDiscovered = node["thisMonthDiscovered"].as<bool>(_thisMonthDiscovered);
	_treaties = node["treaties"].as<std::vector<std::string>>(_treaties);
}

/**
 * Saves the event to YAML.
 * @return YAML node.
 */ 
YAML::Node DiplomacyFaction::save() const
{
	YAML::Node node;
	node["name"] = _rule.getName();
	node["reputationScore"] = _reputationScore;
	node["reputationLvL"] = _reputationLvL;
	node["reputationName"] = _reputationName;
	if (_discovered)
	{
		node["discovered"] = _discovered;
	}
	if (_thisMonthDiscovered)
	{
		node["thisMonthDiscovered"] = _thisMonthDiscovered;
	}
	node["treaties"] = _treaties;
	return node;
}

/**
 * Handle Faction logic.
 * @param Game game engine.
 * @param ThinkPeriond - timestep to determine think process
 */
void DiplomacyFaction::think(Game& engine, ThinkPeriod period)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();
	_commandsToProcess.clear();
	_eventsToProcess.clear();
	//lets process out daily duty
	if (period == TIMESTEP_DAILY)
	{
		//check if we discover our faction
		if (!_discovered && save.isResearched(mod.getResearch(_rule.getDiscoverResearch())))
		{
			setDiscovered(true);
			setThisMonthDiscovered(true);
			//spawn celebration event if faction wants it
			if (!_rule.getDiscoverEvent().empty())
			{
				bool success = engine.getMasterMind()->spawnEvent(_rule.getDiscoverEvent());
			}
			// update reputation level for just discovered fraction
			engine.getMasterMind()->updateReputationLvl(this);
			this->setThisMonthDiscovered(true);
			// and if it turns friendly at start we sign help treaty by default
			if (_reputationLvL > 0)
			{
				_treaties.push_back("STR_HELP_TREATY");
			}
		}
		//process treaty logic
		if (std::find(_treaties.begin(), _treaties.end(), "STR_HELP_TREATY") != _treaties.end())
		{
			//load treaty rules
			_commandsToProcess = _rule.getHelpTreatyMissions();
			_eventsToProcess = _rule.getHelpTreatyEvents();
		}

		//generate missions and events
		factionMissionGenerator(engine);
		factionEventGenerator(engine);

	}
}

/**
 * Handle initial processing of factional missions and stores succes cases.
 * @param Game game engine.
 */
void DiplomacyFaction::factionMissionGenerator(Game& engine)
{
	if (!_commandsToProcess.empty())
	{
		const Mod& mod = *engine.getMod();
		SavedGame& save = *engine.getSavedGame();
		if (RNG::percent(_rule.getGenMissionFrequency()))
		{
			for (auto name : _commandsToProcess)
			{
				auto ruleScript = mod.getMissionScript(name);
				auto month = save.getMonthsPassed();
				int loyalty = save.getLoyalty();
				// lets process mission script with own way, first things first
				if (ruleScript->getFirstMonth() <= month &&
					(ruleScript->getLastMonth() >= month || ruleScript->getLastMonth() == -1) &&
					(ruleScript->getMaxRuns() == -1 || ruleScript->getMaxRuns() > save.getAlienStrategy().getMissionsRun(ruleScript->getVarName())) &&
					(month < 1 || ruleScript->getMinScore() <= save.getCurrentScore(month)) &&
					(month < 1 || ruleScript->getMaxScore() >= save.getCurrentScore(month)) &&
					(month < 1 || ruleScript->getMinLoyalty() <= loyalty) &&
					(month < 1 || ruleScript->getMaxLoyalty() >= loyalty) &&
					(month < 1 || ruleScript->getMinFunds() <= save.getFunds()) &&
					(month < 1 || ruleScript->getMaxFunds() >= save.getFunds()) &&
					ruleScript->getMinDifficulty() <= save.getDifficulty() &&
					(ruleScript->getAllowedProcessor() == 0 || ruleScript->getAllowedProcessor() == 2) &&
					!save.getMissionScriptGapped(ruleScript->getType()))
				{
					// level two condition check: make sure we meet any research requirements, if any.
					bool triggerHappy = true;
					for (std::map<std::string, bool>::const_iterator j = ruleScript->getResearchTriggers().begin(); triggerHappy && j != ruleScript->getResearchTriggers().end(); ++j)
					{
						triggerHappy = (save.isResearched(j->first) == j->second);
						if (!triggerHappy)
							continue;
					}
					if (triggerHappy)
					{
						// item requirements
						for (auto& triggerItem : ruleScript->getItemTriggers())
						{
							triggerHappy = (save.isItemObtained(triggerItem.first) == triggerItem.second);
							if (!triggerHappy)
								continue;
						}
					}
					if (triggerHappy)
					{
						// facility requirements
						for (auto& triggerFacility : ruleScript->getFacilityTriggers())
						{
							triggerHappy = (save.isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
							if (!triggerHappy)
								continue;
						}
					}
					// levels one and two passed: insert this command into the array.
					if (triggerHappy)
					{
						_availableMissionScripts.push_back(ruleScript);
					}
				}
			}
		}
	}
}

/**
 * Handle factional events processing.
 * @param Game game engine.
 */
void DiplomacyFaction::factionEventGenerator(Game& engine)
{
	if (!_eventsToProcess.empty())
	{
		const Mod& mod = *engine.getMod();
		SavedGame& save = *engine.getSavedGame();
		if (RNG::percent(_rule.getGenEventFrequency()))
		{
			for (auto name : _eventsToProcess)
			{
				auto ruleScript = mod.getEventScript(name);
				auto month = save.getMonthsPassed();
				int loyalty = save.getLoyalty();
				if (ruleScript->getFirstMonth() <= month &&
					(ruleScript->getLastMonth() >= month || ruleScript->getLastMonth() == -1) &&
					(month < 1 || ruleScript->getMinScore() <= save.getCurrentScore(month)) &&
					(month < 1 || ruleScript->getMaxScore() >= save.getCurrentScore(month)) &&
					(month < 1 || ruleScript->getMinLoyalty() <= loyalty) &&
					(month < 1 || ruleScript->getMaxLoyalty() >= loyalty) &&
					(month < 1 || ruleScript->getMinFunds() <= save.getFunds()) &&
					(month < 1 || ruleScript->getMaxFunds() >= save.getFunds()) &&
					ruleScript->getMinDifficulty() <= save.getDifficulty() &&
					(ruleScript->getAllowedProcessor() == 0 || ruleScript->getAllowedProcessor() == 2))
				{
					// level two condition check: make sure we meet any research requirements, if any.
					bool triggerHappy = true;
					for (std::map<std::string, bool>::const_iterator j = ruleScript->getResearchTriggers().begin(); triggerHappy && j != ruleScript->getResearchTriggers().end(); ++j)
					{
						triggerHappy = (save.isResearched(j->first) == j->second);
						if (!triggerHappy)
							continue;
					}
					if (triggerHappy)
					{
						// item requirements
						for (auto& triggerItem : ruleScript->getItemTriggers())
						{
							triggerHappy = (save.isItemObtained(triggerItem.first) == triggerItem.second);
							if (!triggerHappy)
								continue;
						}
					}
					if (triggerHappy)
					{
						// facility requirements
						for (auto& triggerFacility : ruleScript->getFacilityTriggers())
						{
							triggerHappy = (save.isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
							if (!triggerHappy)
								continue;
						}
					}
					// ok, we still want event from this script, now let`s actually choose one.
					if (triggerHappy)
					{
						std::vector<const RuleEvent*> toBeGenerated;

						// 1. sequentially generated one-time events (cannot repeat)
						{
							std::vector<std::string> possibleSeqEvents;
							for (auto& seqEvent : ruleScript->getOneTimeSequentialEvents())
							{
								if (!save.wasEventGenerated(seqEvent))
									possibleSeqEvents.push_back(seqEvent); // insert
							}
							if (!possibleSeqEvents.empty())
							{
								auto eventRules = mod.getEvent(possibleSeqEvents.front(), true); // take first
								toBeGenerated.push_back(eventRules);
							}
						}

						// 2. randomly generated one-time events (cannot repeat)
						{
							WeightedOptions possibleRngEvents;
							WeightedOptions tmp = ruleScript->getOneTimeRandomEvents(); // copy for the iterator, because of getNames()
							possibleRngEvents = tmp; // copy for us to modify
							for (auto& rngEvent : tmp.getNames())
							{
								if (save.wasEventGenerated(rngEvent))
									possibleRngEvents.set(rngEvent, 0); // delete
							}
							if (!possibleRngEvents.empty())
							{
								auto eventRules = mod.getEvent(possibleRngEvents.choose(), true); // take random
								toBeGenerated.push_back(eventRules);
							}
						}

						// 3. randomly generated repeatable events
						{
							auto eventRules = mod.getEvent(ruleScript->generate(save.getMonthsPassed()), false);
							if (eventRules)
							{
								toBeGenerated.push_back(eventRules);
							}
						}

						// 4. generate
						for (auto eventRules : toBeGenerated)
						{
							engine.getMasterMind()->spawnEvent(eventRules->getName());
						}
					}
				}
			}
		}
	}
}

}
