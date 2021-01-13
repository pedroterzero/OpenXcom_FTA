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
#include <algorithm>
#include "../fmath.h"
#include "../Engine/Game.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleEventScript.h"
#include "../Mod/RuleMissionScript.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleSoldier.h"
#include "../Mod/RuleCraft.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Savegame/AlienStrategy.h"
#include "../Savegame/ItemContainer.h"
#include "../FTA/MasterMind.h"

namespace OpenXcom
{

DiplomacyFaction::DiplomacyFaction(const Mod* mod, const RuleDiplomacyFaction &rule) :
					_mod(mod), _rule(rule), _reputationScore(0), _reputationLvL(0), _funds(0), _power(0), _vigilance(0),
					_discovered(false), _thisMonthDiscovered(false), _repLvlChanged(false)
{
	_items = new ItemContainer();
	_staff = new FactionalContainer();
}

DiplomacyFaction::~DiplomacyFaction()
{
	for (std::vector<FactionalResearch*>::iterator i = _research.begin(); i != _research.end(); ++i)
	{
		delete* i;
	}
	delete _items;
	delete _staff;
}

/**
 * Loads the Diplomacy Faction from YAML.
 * @param node The YAML node containing the data.
 */
void DiplomacyFaction::load(const YAML::Node &node)
{
	_reputationScore = node["reputationScore"].as<int>(_reputationScore);
	_reputationLvL = node["reputationLvL"].as<int>(_reputationLvL);
	_reputationName = node["reputationName"].as<std::string>(_reputationName);
	_repLvlChanged = node["repLvlChanged"].as<bool>(_repLvlChanged);
	_funds = node["funds"].as<int>(_funds);
	_power = node["power"].as<int>(_power);
	_vigilance = node["vigilance"].as<int>(_vigilance);
	_discovered = node["discovered"].as<bool>(_discovered);
	_thisMonthDiscovered = node["thisMonthDiscovered"].as<bool>(_thisMonthDiscovered);
	_treaties = node["treaties"].as<std::vector<std::string>>(_treaties);
	_unlockedResearches = node["unlockedResearches"].as<std::vector<std::string>>(_unlockedResearches);
	_items->load(node["items"]);
	_staff->load(node["staff"]);
	for (YAML::const_iterator i = node["research"].begin(); i != node["research"].end(); ++i)
	{
		std::string name = (*i)["name"].as<std::string>();
		if (_mod->getResearch(name))
		{
			FactionalResearch* r = new FactionalResearch(_mod->getResearch(name), this);
			r->load(*i);
			_research.push_back(r);
		}
		else
		{
			Log(LOG_ERROR) << "Failed to load research " << name;
		}
	}
	_dailyRepScore = node["dailyRepScore"].as<std::vector<int>>(_dailyRepScore);
}

/**
 * Saves the Diplomacy Faction to YAML.
 * @return YAML node.
 */ 
YAML::Node DiplomacyFaction::save() const
{
	YAML::Node node;
	node["name"] = _rule.getName();
	node["reputationScore"] = _reputationScore;
	node["reputationLvL"] = _reputationLvL;
	node["reputationName"] = _reputationName;
	if (_repLvlChanged)
	{
		node["repLvlChanged"] = _repLvlChanged;
	}
	node["funds"] = _funds;
	node["power"] = _power;
	node["vigilance"] = _vigilance;
	if (_discovered)
	{
		node["discovered"] = _discovered;
	}
	if (_thisMonthDiscovered)
	{
		node["thisMonthDiscovered"] = _thisMonthDiscovered;
	}
	node["treaties"] = _treaties;
	node["unlockedResearches"] = _unlockedResearches;
	node["items"] = _items->save();
	node["staff"] = _staff->save();
	for (std::vector<FactionalResearch*>::const_iterator i = _research.begin(); i != _research.end(); ++i)
	{
		node["research"].push_back((*i)->save());
	}
	node["dailyRepScore"] = _dailyRepScore;
	return node;
}

/**
 * Updates reputation score based on incoming value and handle simple reaction to it.
 * @param change value that should be added to reputation score.
 */
void DiplomacyFaction::updateReputationScore(int change)
{
	if (change == 0)
		return;

	_reputationScore += change;
	_dailyRepScore.push_back(change);
}

/**
 * Removes research projet's name to a faction's list of unlocked researches.
 * @param research name of research project.
 */
void DiplomacyFaction::disableResearch(const std::string& research)
{
	bool erased = false;
	std::vector<std::string>::iterator r = std::find(_unlockedResearches.begin(), _unlockedResearches.end(), research);
	if (r != _unlockedResearches.end())
	{
		_unlockedResearches.erase(r);
		erased = true;
	}
	if (!erased) { Log(LOG_ERROR) << "Research project  named " << research << " was not deleted from <unlockedResearches> list!"; }
}

/**
 * Main handler of Faction logic.
 * @param Game game engine.
 * @param ThinkPeriod - timestep to determine think process
 */
void DiplomacyFaction::think(Game& engine, ThinkPeriod period)
{
	SavedGame& save = *engine.getSavedGame();
	MasterMind& mind = *engine.getMasterMind();
	_commandsToProcess.clear();
	_eventsToProcess.clear();

	//let's process out daily duty
	if (period == TIMESTEP_DAILY)
	{
		//check if we discover our faction
		if (!_discovered && save.isResearched(_mod->getResearch(_rule.getDiscoverResearch())))
		{
			setDiscovered(true);
			setThisMonthDiscovered(true);
			//spawn celebration event if faction wants it
			if (!_rule.getDiscoverEvent().empty())
			{
				bool success = mind.spawnEvent(_rule.getDiscoverEvent());
			}
			// update reputation level for just discovered fraction
			mind.updateReputationLvl(this, false);
			this->setThisMonthDiscovered(true);
			// and if it turns friendly at start we sign help treaty by default
			if (_reputationLvL > 0)
			{
				_treaties.push_back("STR_HELP_TREATY");
			}
		}
		//check if we need to react reputation score change
		if (!_dailyRepScore.empty())
		{
			processDailyReputation(engine);
		}

		//managment time
		handleRestock();
		manageStaff();
		auto reqFunds = managePower(save.getMonthsPassed(), _mod->getDefaultFactionPowerCost());
		handleResearch(reqFunds);

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
 * Handle daily reputation change and immidiate reaction to it.
 * @param Game game engine.
 */
void DiplomacyFaction::processDailyReputation(Game& engine)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();
	MasterMind& mind = *engine.getMasterMind();
	int dailyReputation = 0;

	for (auto& n : _dailyRepScore)
	{
		dailyReputation += n;
	}
	_dailyRepScore.clear();

	if (dailyReputation)
	{
		const std::map<int, std::string>* repLevels = mod.getReputationLevels();
		if (repLevels)
		{
			int nextRepScore = 0;
			std::vector<std::string> events;
			bool needUpdate = false;

			for (auto it = repLevels->begin(); it != repLevels->end(); ++it)
			{
				if (it->second == _reputationName)
				{
					if (dailyReputation > 0)
					{
						nextRepScore = it++->first;
						if (nextRepScore < _reputationScore + dailyReputation)
						{
							events = _rule.getHappyEvents();
							needUpdate = true;
						}
					}
					else
					{
						nextRepScore = it--->first;
						if (nextRepScore > _reputationScore + dailyReputation)
						{
							events = _rule.getAngryEvents();
							needUpdate = true;
							//we break all friendly-like treaties in this case
							_treaties.clear();
						}
					}
					break;
				}
			}

			if (needUpdate)
			{
				mind.updateReputationLvl(this, false);
				_repLvlChanged = true;
				if (!events.empty())
				{
					mind.spawnEvent(events.at(RNG::generate(0, events.size() - 1)));
				}
			}
		}
	}
}

/**
 * Handle initial processing of factional missions and stores success cases.
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

/**
 * Handle balancing of Faction's items, buying or selling based on current situation.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::handleRestock()
{
	std::map<std::string, double> buyList;

	for (auto it = _mod->getItemsList().begin(); it != _mod->getItemsList().end(); ++it)
	{
		RuleItem* ruleItem = _mod->getItem(*(it));
		int cost = ruleItem->getBuyCost();

		// first, we see if we can handle this item
		auto requirements = ruleItem->getRequirements();
		auto reqBuy = ruleItem->getBuyRequirements();
		bool usage = false;
		bool purchase = false;
		if (!requirements.empty())
		{
			for (auto r = requirements.begin(); r != requirements.end(); ++r)
			{
				if (std::find(_unlockedResearches.begin(), _unlockedResearches.end(), (*r)->getName()) != _unlockedResearches.end())
				{
					usage = true;
					break;
				}
			}
			if (!usage)
			{
				continue;
			}
		}
		if (!reqBuy.empty())
		{
			for (auto r = reqBuy.begin(); r != reqBuy.end(); ++r)
			{
				if (std::find(_unlockedResearches.begin(), _unlockedResearches.end(), (*r)->getName()) != _unlockedResearches.end())
				{
					purchase = true;
					break;
				}
			}
			if (!purchase)
			{
				continue;
			}
		}

		// ok, we know what is that toy, move on
		double wishWeight = 1;
		for (auto k = _rule.getWishList().begin(); k != _rule.getWishList().end(); ++k)
		{
			if (k->first == (*it))
			{
				wishWeight = k->second;
				break;
			}
		}
		// calculate relative weight of that item
		double rWeight = wishWeight / cost;
		buyList.insert(std::make_pair(*(it), rWeight));
	}

	// now let's calculate absolute weight from sum of relative values
	double rWeightSum = 0;
	for (auto i = buyList.begin(); i != buyList.end(); ++i)
	{
		rWeightSum += (*i).second;
	}

	// finally, we can purchase or sell things.
	Log(LOG_DEBUG) << "Handling restock, Faction:  " << this->getRules().getName() << " has funds: " << _funds << " and power: " << _power << "."; //#CLEARLOGS

	for (auto i = buyList.begin(); i != buyList.end(); ++i)
	{
		int toBuy = floor(((*i).second / rWeightSum * _power * _rule.getStockMod()) - _items->getItem((*i).first));

		if (toBuy)
		{
			int cost = _mod->getItem((*i).first)->getBuyCost();
			if (toBuy > 0 || _funds > 0)
			{
				_items->addItem((*i).first, toBuy);
				_funds -= toBuy * cost;
				Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " is buying items " << (*i).first << ": " << toBuy << " because of coef: " << (*i).second; //#CLEARLOGS
			}
			else
			{
				_items->removeItem((*i).first, toBuy);
				_funds += toBuy * cost; //yes, faction can sell items with purchase cost, or balancing resources would go crazy.
				Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " is selling items " << (*i).first << ": " << toBuy << " because of coef: " << (*i).second; //#CLEARLOGS
			}
		}
	}
	Log(LOG_DEBUG) << "Restock finished, Faction:  " << this->getRules().getName() << " now has funds: " << _funds << "."; //#CLEARLOGS
}

/**
 * Handle managing of Faction's staff and non-item equipment.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::manageStaff()
{
	std::map<std::string, double> buyList;

	// handle soldiers
	const std::vector<std::string>& soldiers = _mod->getSoldiersList();
	for (std::vector<std::string>::const_iterator i = soldiers.begin(); i != soldiers.end(); ++i)
	{

		RuleSoldier* rule = _mod->getSoldier(*i);
		
		if (rule->getBuyCost() != 0 && isResearched(rule->getRequirements()))
		{
			buyList.insert(std::make_pair((*i), rule->getBuyCost()));
		}
	}
	
	// handle STR_SCIENTIST
	if (isResearched("STR_SCIENTIST"))
	{
		buyList.insert(std::make_pair("STR_SCIENTIST", _mod->getHireScientistCost()));
	}

	// handle STR_ENGINEER
	if (isResearched("STR_ENGINEER"))
	{
		buyList.insert(std::make_pair("STR_ENGINEER", _mod->getHireEngineerCost()));
	}

	// handle crafts
	const std::vector<std::string>& crafts = _mod->getCraftsList();
	for (std::vector<std::string>::const_iterator i = crafts.begin(); i != crafts.end(); ++i)
	{

		RuleCraft* rule = _mod->getCraft(*i);

		if (rule->getBuyCost() != 0 && isResearched(rule->getRequirements()))
		{
			buyList.insert(std::make_pair((*i), rule->getBuyCost()));
		}
	}

	for (auto j = buyList.begin(); j != buyList.end(); ++j)
	{
		double weight = 0;
		for (auto k = _rule.getWishList().begin(); k != _rule.getWishList().end(); ++k)
		{
			if (k->first == (*j).first)
			{

				weight = k->second;
				break;
			}
		}

		if (weight)
		{
			
			int toBuy = floor((_power / weight) - _staff->getItem((*j).first));

			if (toBuy)
			{
				int cost = (*j).second;
				if (toBuy > 0 || _funds > 0)
				{
					_staff->addItem((*j).first, toBuy);
					_funds -= toBuy * cost;
					Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " is buying staff " << (*j).first << ": " << toBuy << " because of coef: " << floor(_power / weight); //#CLEARLOGS
				}
				else
				{
					_staff->removeItem((*j).first, toBuy);
					_funds += toBuy * cost; //yes, faction can sell items with purchase cost, or balancing resources would go crazy.
					Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " is selling staff " << (*j).first << ": " << toBuy << " because of coef: " << floor(_power / weight); //#CLEARLOGS
				}
			}
		}

	}
}

/**
 * Handle balancing of Faction's power.
 * @param mod rulesets to get constant data.
 */
int64_t DiplomacyFaction::managePower(int month, int baseCost)
{
	int powerHungry = _rule.getPowerHungry();
	int64_t reqFunds = _power * ((static_cast<int64_t>(month / month * 14) * powerHungry) + (powerHungry / 2.33));
	int64_t powerCost = baseCost + static_cast<int64_t>((baseCost * (month - 1)) * 0.017);
	int dPower = 0;

	if (_funds < 0)
	{
		if (_funds < -reqFunds / 1.3)
		{
			int i = _funds + (reqFunds / 1.3);
			dPower = static_cast<int>((-_funds + (reqFunds / 1.3)) / powerCost * 0.9);
			if (dPower > _power)
			{
				dPower = _power;
			}
			_power -= dPower;
			Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " updating power to: " << _power << " with dPower value: " << dPower << " because its funds: " << _funds << " are below limit: " << reqFunds; //#CLEARLOGS
			_funds += static_cast<int>(dPower * powerCost * 0.9);
		}
		else
		{
			return reqFunds;
		}
	}

	if (_vigilance > 0)
	{
		if (_funds > reqFunds)
		{
			int cost = static_cast<int>((_funds - reqFunds) / powerCost);
			dPower = std::min(cost, _vigilance);
			_power += dPower;
			_funds -= dPower * powerCost;
			Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " updating power to: " << _power << " with dPower value: " << dPower << " because it has vigilance: " << _vigilance << " and funds are above reqFunds value: " << reqFunds; //#CLEARLOGS
		}
		else
		{
			Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " has vigilance value : " << _vigilance << ", but can't update power because funds value: " << _funds << " below reqFunds: " << reqFunds; //#CLEARLOGS
		}
	}

	return reqFunds;
}

/**
 * Handle managing of Faction's staff and non-item equipment.
 * @param mod rulesets to get constant data.
 */
void DiplomacyFaction::handleResearch(int64_t reqFunds)
{
	if (_staff->getItem("STR_SCIENTIST") > 0 || _funds > reqFunds)
	{
		std::vector<std::pair<RuleResearch*, int>> researchList;

		for (auto i = _mod->getResearchList().begin(); i != _mod->getResearchList().end(); ++i)
		{
			RuleResearch* rRule = _mod->getResearch((*i));

			if (rRule == nullptr)
				continue;

			if (isResearched(rRule->getDependencies()))// this one effectively splits xcom and factional research trees.
			{
				if (rRule->needItem())
				{
					if (_items->getItem(*i) > 0)
					{
						int priority = rRule->getPoints() / rRule->getCost();
						researchList.push_back(std::make_pair(rRule, priority));
						Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " has potential research : " << rRule->getName() << ", processing! It has priority: " << priority; //#CLEARLOGS
					}
				}
			}
		}

		if (!researchList.empty())
		{
			//std::sort(researchList.begin(), researchList.end(), [&](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return (a.second < b.second); });

			RuleResearch* choice = nullptr;

			for (auto j = researchList.begin(); j != researchList.end(); ++j)
			{
				choice = (*j).first;
				int researchCost = round(choice->getCost() / 24 * 10); // balance cost back to days (FtA uses hours) and icrease by 10 as factions are slow on researching
				int scienceBaseCost = _rule.getScienceBaseCost();

				if (_funds > ((reqFunds * 2 / 3) - (researchCost * scienceBaseCost)))
				{



					Log(LOG_DEBUG) << "Faction:  " << this->getRules().getName() << " has chosen research : " << choice->getName()
						<< " with priority: " << researchList.front().second; //#CLEARLOGS
				}


				
			}


		}
	}

	
	


}

bool DiplomacyFaction::isResearched(const std::string& name) const
{
	if (std::find(_unlockedResearches.begin(), _unlockedResearches.end(), name) != _unlockedResearches.end())
	{
		return true;
	}
	return false;
}

bool DiplomacyFaction::isResearched(const RuleResearch* rule) const
{
	if (std::find(_unlockedResearches.begin(), _unlockedResearches.end(), rule->getName()) != _unlockedResearches.end())
	{
		return true;
	}
	return false;
}

bool DiplomacyFaction::isResearched(const std::vector<std::string>& names) const
{
	if (names.empty())
		return true;

	for (const std::string& r : names)
	{
		if (!isResearched(r))
		{
			return false;
		}
	}

	return false;
}

bool DiplomacyFaction::isResearched(const std::vector<const RuleResearch*>& rules) const
{
	if (rules.empty())
		return true;

	for (auto r : rules)
	{
		if (!isResearched(r))
		{
			return false;
		}
	}

	return false;
}



/**
 * Initializes an FactionalContainer with no contents.
 */
FactionalResearch::FactionalResearch(RuleResearch* rule, DiplomacyFaction* faction): _rule(rule), _faction(faction)
{
}

/**
 *
 */
FactionalResearch::~FactionalResearch()
{
}

/**
 * Loads the Diplomacy Faction from YAML.
 * @param node The YAML node containing the data.
 */
void FactionalResearch::load(const YAML::Node& node)
{
	_scientists = node["scientists"].as<int>(_scientists);
	_timeLeft = node["timeLeft"].as<int>(_timeLeft);
}

/**
 * Saves the Factional Research to YAML.
 * @return YAML node.
 */
YAML::Node FactionalResearch::save() const
{
	YAML::Node node;
	node["name"] = _rule->getName();
	node["scientists"] = _scientists;
	node["timeLeft"] = _timeLeft;

	return node;
}






/**
 * Initializes an FactionalContainer with no contents.
 */
FactionalContainer::FactionalContainer()
{
}

/**
 *
 */
FactionalContainer::~FactionalContainer()
{
}

/**
 * Loads the FactionalContainer from a YAML file.
 * @param node YAML node.
 */
void FactionalContainer::load(const YAML::Node& node)
{
	_qty = node.as< std::map<std::string, int> >(_qty);
}

/**
 * Saves the FactionalContainer to a YAML file.
 * @return YAML node.
 */
YAML::Node FactionalContainer::save() const
{
	YAML::Node node;
	node = _qty;
	return node;
}

/**
 * Adds an entity amount to the container.
 * @param id entity ID.
 * @param qty entity quantity.
 */
void FactionalContainer::addItem(const std::string& id, int qty)
{
	if (id.empty())
	{
		return;
	}
	_qty[id] += qty;
}


/**
 * Removes an entity amount from the container.
 * @param id entity ID.
 * @param qty entity quantity.
 */
void FactionalContainer::removeItem(const std::string& id, int qty)
{
	if (id.empty())
	{
		return;
	}
	auto it = _qty.find(id);
	if (it == _qty.end())
	{
		return;
	}

	if (qty < it->second)
	{
		it->second -= qty;
	}
	else
	{
		_qty.erase(it);
	}
}

/**
 * Returns the quantity of an entity in the container.
 * @param id entity ID.
 * @return entity quantity.
 */
int FactionalContainer::getItem(const std::string& id) const
{
	if (id.empty())
	{
		return 0;
	}

	auto it = _qty.find(id);
	if (it == _qty.end())
	{
		return 0;
	}
	else
	{
		return it->second;
	}
}

}
