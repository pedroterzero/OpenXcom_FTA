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
#include "RuleDiplomacyFaction.h"
#include "../Engine/RNG.h"
#include "../fmath.h"

//namespace YAML
//{
//	template<>
//	struct convert<OpenXcom::FactionMarketList>
//	{
//		static Node encode(const OpenXcom::FactionMarketList& rhs)
//		{
//			Node node;
//			node = rhs.reputationList;
//			return node;
//		}
//
//		static bool decode(const Node& node, OpenXcom::FactionMarketList& rhs)
//		{
//			if (!node.IsSequence())
//				return false;
//
//			rhs.reputationList = node.as< std::map<std::string, int> >(rhs.reputationList);
//			return true;
//		}
//	};
//}

namespace OpenXcom
{

RuleDiplomacyFaction::RuleDiplomacyFaction(const std::string &name) :
			_name(name), _description("NONE"), _background("BACK13.SCR"), _cardBackground("BACK13.SCR"),
			_genMissionFrequency(0), _genEventFrequency(0),
			_sellPriceFactor(0), _buyPriceFactor(0), _repPriceFactor(0), _stockMod(0.1),
			_startingReputation(0), _startingFunds(0), _startingPower(0), _powerHungry(10000), _scienceBaseCost(2000)
{
}

/**
 * Loads the event definition from YAML.
 * @param node YAML node.
 */
void RuleDiplomacyFaction::load(const YAML::Node &node)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent);
	}
	_name = node["name"].as<std::string>(_name);
	_description = node["description"].as<std::string>(_description);
	_background = node["background"].as<std::string>(_background);
	_cardBackground = node["cardBackground"].as<std::string>(_cardBackground); 
	_discoverResearch = node["discoverResearch"].as<std::string>(_discoverResearch);
	_discoverEvent = node["discoverEvent"].as<std::string>(_discoverEvent);
	_helpTreatyMissions = node["helpTreatyMissions"].as<std::vector<std::string>>(_helpTreatyMissions);
	_helpTreatyEvents = node["helpTreatyEvents"].as<std::vector<std::string>>(_helpTreatyEvents);
	_genMissionFrequency = node["genMissionFreq"].as<int>(_genMissionFrequency);
	_genEventFrequency = node["genEventFreq"].as<int>(_genEventFrequency);

	_happyEvents = node["happyEvents"].as<std::vector<std::string>>(_happyEvents);
	_angryEvents = node["angryEvents"].as<std::vector<std::string>>(_angryEvents);

	_sellPriceFactor = node["sellPriceFactor"].as<int>(_sellPriceFactor);
	_buyPriceFactor = node["buyPriceFactor"].as<int>(_buyPriceFactor);
	_repPriceFactor = node["repPriceFactor"].as<int>(_repPriceFactor);
	_stockMod = node["stockMod"].as<int>(_stockMod);
	_stockMod = node["stockMod"].as<int>(_stockMod);
	_wishList = node["wishList"].as<std::map<std::string, double>>(_wishList);
	_powerHungry = node["powerHungry"].as<int>(_powerHungry);
	_scienceBaseCost = node["scienceBaseCost"].as<int>(_scienceBaseCost);

	_startingReputation = node["startingReputation"].as<int>(_startingReputation);
	_startingFunds = node["startingFunds"].as<int>(_startingFunds);
	_startingPower = node["startingPower"].as<int>(_startingPower);
	_startingItems = node["startingItems"].as<std::map<std::string, int>>(_startingItems);
	_startingStaff = node["startingStaff"].as<std::map<std::string, int>>(_startingStaff);
	_startingResearches = node["startingResearches"].as<std::vector<std::string>>(_startingResearches);
}

}
