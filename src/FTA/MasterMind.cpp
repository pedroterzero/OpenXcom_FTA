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

#include "MasterMind.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <climits>
#include <functional>
#include "../Engine/RNG.h"
#include "../Engine/Game.h"
#include "../Engine/Logger.h"
#include "../Engine/Exception.h"
#include "../Engine/State.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Language.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Mod/RuleArcScript.h"
#include "../Mod/RuleEventScript.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleMissionScript.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleAlienMission.h"
#include "../Mod/RuleGlobe.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/RuleManufacture.h"
#include "../Mod/RuleDiplomacyFaction.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/DiplomacyFaction.h"
#include "../Savegame/CovertOperation.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDiary.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/Production.h"
#include "../Savegame/MissionSite.h"
#include "../Savegame/AlienBase.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Savegame/AlienStrategy.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/FactionalContainer.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Geoscape/Globe.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../fmath.h"
#include "../fallthrough.h"


namespace OpenXcom
{

MasterMind::MasterMind(Game* engine): _game(engine)
{
	
}
MasterMind::~MasterMind()
{
}

/**
* Handle additional operations in new FTA game generation.
* @param eventName - string with rules name of the event.
* @return true is event was generater successfully.
*/
void MasterMind::newGameHelper(int diff, GeoscapeState* gs)
{
	SavedGame* save = _game->getSavedGame();
	Mod* mod = _game->getMod();
	Base* base = save->getBases()->at(0);
	double lon, lat;
	lon = RNG::generate(0.20, 0.22); //#FINNIKTODO random array here
	lat = RNG::generate(-0.832, -0.87) ; //#FINNIKTODO random array here
	base->setLongitude(lon);
	base->setLatitude(lat);
	std::string baseName = _game->getLanguage()->getString("STR_LAST_STAND"); //#FINNIKTODO random array here
	base->setName(baseName);
	gs->getGlobe()->center(lon, lat);

	for (std::vector<Craft*>::iterator i = base->getCrafts()->begin(); i != base->getCrafts()->end(); ++i)
	{
		(*i)->setLongitude(lon);
		(*i)->setLatitude(lat);
	}
	//spawn regional MIB HQ
	AlienDeployment* aBaseDeployment = mod->getDeployment("STR_INITIAL_REGIONAL_HQ");
	AlienBase* aBase = new AlienBase(aBaseDeployment, 0);
	aBase->setId(save->getId(aBaseDeployment->getMarkerName()));
	aBase->setAlienRace(aBaseDeployment->getRace());
	aBase->setLongitude(lon + RNG::generate(0.20, 0.26)); //#FINNIKTODO random array here
	aBase->setLatitude(lat - RNG::generate(0.04, 0.06)); //#FINNIKTODO random array here
	aBase->setDiscovered(false);
	save->getAlienBases()->push_back(aBase);

	//init the Game
	gs->init();

	//init Factions
	for (std::vector<std::string>::const_iterator i = mod->getDiplomacyFactionList()->begin(); i != mod->getDiplomacyFactionList()->end(); ++i)
	{
		RuleDiplomacyFaction* factionRules = mod->getDiplomacyFaction(*i);
		DiplomacyFaction* faction = new DiplomacyFaction(mod, factionRules->getName());

		if (factionRules->getDiscoverResearch().empty() || save->isResearched(mod->getResearch(factionRules->getDiscoverResearch())))
		{
			faction->setDiscovered(true);
		}

		// set up starting values
		faction->setReputationScore(factionRules->getStartingReputation());
		updateReputationLvl(faction);
		faction->setFunds(factionRules->getStartingFunds());
		faction->setPower(factionRules->getStartingPower()); //we always start with 0 vigilance
		for (auto research : factionRules->getStartingResearches())
		{
			faction->unlockResearch(research);
		}

		// populate faction item stores and staff
		auto items = faction->getPublicItems();
		for (auto &item : factionRules->getStartingItems())
		{
			RuleItem* itemRule = _game->getMod()->getItem(item.first);
			if (itemRule)
			{
				items->addItem(itemRule, item.second);
			}
			else
			{
				throw Exception("Error in FTA game initialization process: fails to add item " + item.first + " for faction " + factionRules->getName() +
					" ; no item ruleset defined!");
			}
		}
		for (auto &staff : factionRules->getStartingStaff())
		{
			faction->getStaffContainer()->addItem(staff.first, staff.second);
		}

		// finish faction initialization process
		save->getDiplomacyFactions().push_back(faction);
	}

	//adjust funding
	int funds = mod->getInitialFunding();
	funds = funds * 1000 + static_cast<int>(RNG::generate(-1258, 6365)); 
	save->setFunds(funds);

	//start base defense mission
	SavedBattleGame* bgame = new SavedBattleGame(mod, _game->getLanguage());
	_game->getSavedGame()->setBattleGame(bgame);
	bgame->setMissionType("STR_BASE_DEFENSE");
	BattlescapeGenerator bgen = BattlescapeGenerator(_game);
	bgen.setBase(base);
	bgen.setAlienCustomDeploy(mod->getDeployment("STR_INITIAL_BASE_DEFENSE"));
	bgen.setWorldShade(0);
	bgen.run();
	_game->pushState(new BriefingState(0, base));
}

/**
* Process event script from different sources
* @param engine - Game.
* @param scripts - a vector of event script's string IDs.
* @param source is the reason we are running process (mothly, factional, xcom).
*/
void MasterMind::eventScriptProcessor(Game& engine, std::vector<std::string> scripts, ProcessorSource source)
{
	const Mod& mod = *engine.getMod();
	SavedGame& save = *engine.getSavedGame();

	if (!scripts.empty())
	{
		for (auto& name : scripts)
		{
			auto ruleScript = mod.getEventScript(name);
			int allowedProcessor = ruleScript->getAllowedProcessor();
			// check allowed processor first!
			if ((source == MOTHLY && allowedProcessor != 0) ||
				(source == FACTIONAL && allowedProcessor != 1) ||
				(source == XCOM && allowedProcessor != 2))
			{
				continue; //we should skip that!
			}
			auto month = save.getMonthsPassed();
			int loyalty = save.getLoyalty();
			if (ruleScript->getFirstMonth() <= month &&
				(ruleScript->getLastMonth() >= month || ruleScript->getLastMonth() == -1) &&
				(ruleScript->getMinScore() <= save.getCurrentScore(month)) &&
				(ruleScript->getMaxScore() >= save.getCurrentScore(month)) &&
				(ruleScript->getMinLoyalty() <= loyalty) &&
				(ruleScript->getMaxLoyalty() >= loyalty) &&
				(ruleScript->getMinFunds() <= save.getFunds()) &&
				(ruleScript->getMaxFunds() >= save.getFunds()) &&
				ruleScript->getMinDifficulty() <= save.getDifficulty() &&
				ruleScript->getMaxDifficulty() >= save.getDifficulty() &&
				!save.getEventScriptGapped(ruleScript->getType()))
			{
				// level two condition check: make sure we meet any research requirements, if any.
				bool triggerHappy = true;
				for (std::map<std::string, bool>::const_iterator j = ruleScript->getResearchTriggers().begin(); triggerHappy && j != ruleScript->getResearchTriggers().end(); ++j)
				{
					triggerHappy = (save.isResearched(j->first) == j->second);
					if (!triggerHappy)
						continue;
				}

				// reputation requirements
				if (triggerHappy)
				{
					if (!ruleScript->getReputationRequirments().empty())
					{
						triggerHappy = false;
						for (auto& triggerFaction : ruleScript->getReputationRequirments())
						{
							for (auto& faction : engine.getSavedGame()->getDiplomacyFactions())
							{
								if (faction->getRules()->getName() == triggerFaction.first)
								{
									if (faction->getReputationLevel() >= triggerFaction.second)
									{
										triggerHappy = true;
									}
								}
							}
						}
						if (!triggerHappy)
							continue;
					}
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
					bool generated = false;
					for (auto eventRules : toBeGenerated)
					{
						if (spawnEvent(eventRules->getName()))
						{
							generated = true;
						}
					}
					// 4a. if needed any of events were generated, we mark this with gap timer.
					if (generated)
					{
						int timer = ruleScript->getSpawnGap();
						timer += RNG::generate(0, ruleScript->getRandomSpawnGap());
						if (timer > 0)
						{
							_game->getSavedGame()->setEventScriptGapTimer(ruleScript->getType(), timer);
						}
					}
				}
			}
		}
	}
}

/**
* Handle request for generation of Geoscape Event.
* @param eventName - string with rules name of the event.
* @return true is event was generater successfully.
*/
bool MasterMind::spawnEvent(std::string name)
{
	RuleEvent* eventRules = _game->getMod()->getEvent(name);
	if (eventRules == 0)
	{
		throw Exception("Error processing spawning of event: " + name + ", no such rules defined!");
		return false;
	}
	GeoscapeEvent* newEvent = new GeoscapeEvent(*eventRules);
	int minutes = (eventRules->getTimer() + (RNG::generate(0, eventRules->getTimerRandom()))) / 30 * 30;
	if (minutes < 30) minutes = 30; // just in case
	newEvent->setSpawnCountdown(minutes);
	_game->getSavedGame()->getGeoscapeEvents().push_back(newEvent);
	_game->getSavedGame()->addGeneratedEvent(eventRules);

	return true;
}

void MasterMind::updateLoyalty(int score, LoyaltySource source)
{
	double coef = 1;
	std::string reason = "";
	switch (source)
	{
	case OpenXcom::XCOM_BATTLESCAPE:
		coef = _game->getMod()->getLoyaltyCoefBattlescape();
		reason = "XCOM_BATTLESCAPE";
		break;
	case OpenXcom::XCOM_DOGFIGHT:
		coef = _game->getMod()->getLoyaltyCoefDogfight();
		reason = "XCOM_DOGFIGHT";
		break;
	case OpenXcom::XCOM_GEOSCAPE:
		coef = _game->getMod()->getLoyaltyCoefGeoscape();
		reason = "XCOM_GEOSCAPE";
		break;
	case OpenXcom::XCOM_RESEARCH:
		coef = _game->getMod()->getLoyaltyCoefResearch();
		reason = "XCOM_RESEARCH";
		break;
	case OpenXcom::ALIEN_MISSION_DESPAWN:
		coef = _game->getMod()->getLoyaltyCoefAlienMission() * (-1);
		reason = "ALIEN_MISSION_DESPAWN";
		break;
	case OpenXcom::ALIEN_UFO_ACTIVITY:
		coef = _game->getMod()->getLoyaltyCoefUfo() * (-1);
		reason = "ALIEN_UFO_ACTIVITY";
		break;
	case OpenXcom::ALIEN_BASE:
		coef = _game->getMod()->getLoyaltyCoefAlienBase() * (-1);
		reason = "ALIEN_BASE";
		break;
	default:
		break;
	}
	coef = coef / 100;
	int loyalty = _game->getSavedGame()->getLoyalty();
	loyalty += std::round(score * coef);
	Log(LOG_DEBUG) << "Loyalty updating to:  " << loyalty << " from coef: " << coef << " and score value: " << score << " with reason: " << reason; //#CLEARLOGS remove for next release
	_game->getSavedGame()->setLoyalty(loyalty);
}

/**
* Handle calculation of base services (manufacture, labs and craft repair) performance bonus caused by loyalty score.
* @return value of performance bonus.
*/
int MasterMind::getLoyaltyPerformanceBonus()
{
	int bonus = 100;
	if (_game->getMod()->getIsFTAGame())
	{
		int loyalty = _game->getSavedGame()->getLoyalty();
		int cap = _game->getMod()->getLoyaltyPerformanceCap();
		int factor = _game->getMod()->getLoyaltyPerformanceFactor();
		int chance = 0;

		if (loyalty > 0)
		{
			if (loyalty > cap)
			{
				if (RNG::percent(factor / 3))
				{
					bonus = 240;
					return bonus;
				}
				chance = factor;
			}
			else
			{
				chance = (loyalty * factor) / cap;
			}

			if (RNG::percent(chance))
			{
				bonus = 200;
			}
		}
		else
		{
			chance = (-loyalty * factor * 1.5) / cap;

			if (RNG::percent(chance))
			{
				bonus = 50;
			}
			else
			{
				chance = (-loyalty * factor) / cap;

				if (RNG::percent(chance))
				{
					bonus = 0;
				}
			}
		}
	}
	
	return bonus;
}

/**
* Handle updating of faction reputation level based on current score, rules and other conditions.
* @param faction - DiplomacyFaction we are updating.
* @param initial - to define if we are performing updating of reputation due to new game initialisation (default false).
* @return true if reputation level was updated.
*/
bool MasterMind::updateReputationLvl(DiplomacyFaction* faction, bool initial)
{
	int repScore = faction->getReputationScore();
	int curLvl = faction->getReputationLevel();
	bool changed = false;
	std::string repName = "STR_NEUTRAL";
	int newLvl = 3; //STR_NEUTRAL is default resolve

	const std::map<int, std::string>* repLevels = _game->getMod()->getReputationLevels();
	if (repLevels)
	{
		int temp = INT_MIN;
		for (auto& i : *repLevels)
		{
			if (i.first > temp && i.first <= repScore)
			{
				temp = i.first;
				repName = i.second;
			}
		}

		if (repName == "STR_ALLY") newLvl = 6;
		else if (repName == "STR_HONORED") newLvl = 5;
		else if (repName == "STR_FRIENDLY") newLvl = 4;
		else if (repName == "STR_UNFRIENDLY") newLvl = 2;
		else if (repName == "STR_HOSTILE") newLvl = 1;
		else if (repName == "STR_HATED") newLvl = 0;

		if ((curLvl != newLvl && !faction->isThisMonthDiscovered()) || initial)
		{
			faction->setReputationLevel(newLvl);
			faction->setReputationName(repName);
			if (!initial)
			{
				changed = true;
				//#FINNIKTODO add cross-factional relations
			}
		}
	}

	return changed;
}

}
