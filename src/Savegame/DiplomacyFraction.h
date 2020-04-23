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

class RuleDiplomacyFraction;

/**
 * Represents a custom Diplomacy Fraction, spawned at the game start and waiting to be discovered.
 */
class DiplomacyFraction
{
private:
	const RuleDiplomacyFraction &_rule;
	int _reputation;
	bool _discovered;
public:
	/// Creates a blank Diplomacy Fraction.
	DiplomacyFraction(const RuleDiplomacyFraction &rule);
	/// Cleans up the fraction info.
	~DiplomacyFraction();
	/// Loads the fraction from YAML.
	void load(const YAML::Node &node);
	/// Saves the fraction to YAML.
	YAML::Node save() const;
	/// Gets the fraction's ruleset.
	const RuleDiplomacyFraction &getRules() const { return _rule; };
	/// Gets current player's reputation in this fraction.
	int getReputation() const { return _reputation; }
	/// Sets current player's reputation in this fraction.
	void setReputation(int reputation);
	/// Gets corrent reputation level
	int getReputationLevel() const;
	/// Gets corrent reputation level name
	std::string getReputationName() const;
	/// Is this fraction was discovered?
	bool isDiscovered() const { return _discovered; }
	/// Sets fraction's discovered status
	void setDiscovered(bool status);
	/// Handle event spawning schedule.
	//void think();
};

}
