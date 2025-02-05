#pragma once
/*
 * Copyright 2010-2022 OpenXcom Developers.
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
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

class RuleIntelProject;
class RuleIntelStage;
class Soldier;
class Game;
class Globe;
class SavedGame;
class Mod;
class Base;

/**
 Represent a IntelProject
 Contain information about assigned scientist, time already spent and cost of the project.
 */
class IntelProject
{
	const RuleIntelProject * _rules;
	Base* _base;
	std::map<std::string, int> _stageRolls;
	bool _active;
	int _spent;
	int _rolls;
	int _cost;
	/// Get a string describing current progress.
	std::string getState(int progress) const;
public:
	IntelProject(const RuleIntelProject *rule, Base *base, int cost = 0);
	/// Calculates daily progress.
	int getStepProgress(std::map<Soldier*, int> &assignedAgents, Mod *mod, int rating, std::string& description, bool estimate = false);
	/// Game logic. Called every new day.
	bool roll(Game *game, const Globe& globe, int progress, bool &finalRoll);
	const std::vector<const RuleIntelStage*> getAvailableStages(SavedGame* save);
	/// getters and setters
	const std::string getName() const;
	const RuleIntelProject* getRules() const { return _rules; }
	bool getActive() const { return _active; }
	int getRolls() const { return _rolls; }
	int getCost() { return _cost; }
	void setCost(int cost) { _cost = cost; }
	/// load the IntelProject from YAML
	void load(const YAML::Node& node);
	/// save the IntelProject to YAML
	YAML::Node save() const;
};

}
