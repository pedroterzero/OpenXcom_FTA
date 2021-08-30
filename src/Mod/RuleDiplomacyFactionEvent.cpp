/*
 * Copyright 2010-2021 OpenXcom Developers.
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
#include "RuleDiplomacyFactionEvent.h"
#include <climits>

namespace OpenXcom
{

/**
* RuleDiplomacyFactionEvent: the (optional) rules for generating Factional events, to simulate ongoing processes for it's AI.
* Each element is independent, and Faction in its daily think() process will go through all allowed for this faction rules to check if something is allowed to happen.
*/
RuleDiplomacyFactionEvent::RuleDiplomacyFactionEvent(const std::string& type) :
	_type(type), _firstMonth(0), _lastMonth(-1), _executionOdds(100), _minDifficulty(0), _maxDifficulty(4),
	_minPlayerScore(INT_MIN), _maxPlayerScore(INT_MAX), _minPower(INT_MIN), _maxPower(INT_MAX), _powerChange(0), _vigilanceChange(0),
	_minFunds(INT64_MIN), _maxFunds(INT64_MAX), _fundsChange(0)
{
}

/**
* Cleans up the faction event ruleset.
*/
RuleDiplomacyFactionEvent::~RuleDiplomacyFactionEvent()
{
}

/**
* Loads an event script from YAML.
* @param node YAML node.
*/
void RuleDiplomacyFactionEvent::load(const YAML::Node& node)
{
	if (const YAML::Node& parent = node["refNode"])
	{
		load(parent);
	}
	_type = node["type"].as<std::string>(_type);
	_firstMonth = node["firstMonth"].as<int>(_firstMonth);
	_lastMonth = node["lastMonth"].as<int>(_lastMonth);
	_executionOdds = node["executionOdds"].as<int>(_executionOdds);
	_minDifficulty = node["minDifficulty"].as<int>(_minDifficulty);
	_maxDifficulty = node["maxDifficulty"].as<int>(_maxDifficulty);
	_minPlayerScore = node["minPlayerScore"].as<int>(_minPlayerScore);
	_maxPlayerScore = node["maxPlayerScore"].as<int>(_maxPlayerScore);
	_minPower = node["minPower"].as<int>(_minPower);
	_maxPower = node["maxPower"].as<int>(_maxPower);
	_powerChange = node["powerChange"].as<int>(_powerChange);
	_vigilanceChange = node["vigilanceChange"].as<int>(_vigilanceChange);
	_minFunds = node["minFunds"].as<int64_t>(_minFunds);
	_maxFunds = node["maxFunds"].as<int64_t>(_maxFunds);
	_fundsChange = node["fundsChange"].as<int64_t>(_fundsChange);
	_playerResearchTriggers = node["playerResearchTriggers"].as<std::map<std::string, bool> >(_playerResearchTriggers);
	_factionResearchTriggers = node["factionResearchTriggers"].as<std::map<std::string, bool> >(_factionResearchTriggers);
	_itemTriggers = node["itemTriggers"].as<std::map<std::string, bool> >(_itemTriggers);
	_itemsToAdd = node["itemsToAdd"].as<std::map<std::string, int> >(_itemsToAdd);
	_staffToAdd = node["staffToAdd"].as<std::map<std::string, int> >(_staffToAdd);
	_discoveredResearches = node["discoveredResearches"].as<std::vector<std::string>>(_discoveredResearches);
}
}
