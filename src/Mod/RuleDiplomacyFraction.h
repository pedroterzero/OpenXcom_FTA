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
#include <map>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "../Savegame/WeightedOptions.h"

namespace OpenXcom
{
//struct SellingSetEnitity
//{
//	std::string name;
//	int reqReputationLvl;
//	int priceMod;
//
//	SellingSetEnitity() : name(0), reqReputationLvl(0), priceMod(0) { }
//};
/**
 * Represents a custom Geoscape event.
 * Events are spawned using Event Script ruleset.
 */
class RuleDiplomacyFraction
{
private:
	std::string _name, _description, _background, _cardBackground, _music;
	int _startingReputation;
	std::map<std::string, int> _sellingSet;
	//std::vector<SellingSetEnitity> _sellingSet2;
public:
	/// Creates a blank RuleDiplomacyFraction.
	RuleDiplomacyFraction(const std::string &name);
	/// Cleans up the fraction ruleset.
	~RuleDiplomacyFraction() = default;
	/// Loads the fraction definition from YAML.
	void load(const YAML::Node &node);
	/// Gets the fraction's name.
	const std::string &getName() const { return _name; }
	/// Gets the fraction's description.
	const std::string &getDescription() const { return _description; }
	/// Gets the fraction's background sprite name.
	const std::string &getBackground() const { return _background; }
	/// Gets the sprite name, that uses to render diplomacy card.
	const std::string &getCardBackground() const { return _cardBackground; }
	/// Gets the fraction's selling list for purchase state.
	const std::map<std::string, int>&getSellingSet() const { return _sellingSet; }
	//const std::vector<SellingSetEnitity>& getSellingSet2() const { return _sellingSet2; }
};
}

//namespace YAML
//{
//	template<>
//	struct convert<OpenXcom::SellingSetEnitity>
//	{
//		static Node encode(const OpenXcom::SellingSetEnitity& rhs)
//		{
//			Node node;
//			node.push_back(rhs.name);
//			node.push_back(rhs.reqReputationLvl);
//			node.push_back(rhs.priceMod);
//			return node;
//		}
//
//		static bool decode(const Node& node, OpenXcom::SellingSetEnitity& rhs)
//		{
//			if (!node.IsSequence() || node.size() != 3)
//				return false;
//
//			rhs.name = node[0].as<std::string>();
//			rhs.reqReputationLvl = node[1].as<int>();
//			rhs.priceMod = node[2].as<int>();
//			return true;
//		}
//	};
//}
