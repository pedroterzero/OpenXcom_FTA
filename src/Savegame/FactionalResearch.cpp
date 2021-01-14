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
#include "FactionalResearch.h"
#include <assert.h>
#include <algorithm>
#include "../fmath.h"
#include "../Engine/RNG.h"
#include "../Engine/Logger.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleDiplomacyFaction.h"


namespace OpenXcom
{

/**
	* Initializes an FactionalContainer with no contents.
	*/


FactionalResearch::FactionalResearch(const RuleResearch* rule, DiplomacyFaction* faction) : _rule(rule), _faction(faction)
{
}

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

bool FactionalResearch::think()
{
	return false;
}

const std::string& FactionalResearch::getName()
{
	return _rule->getName();
}

}
