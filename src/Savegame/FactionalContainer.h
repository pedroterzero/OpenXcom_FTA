#pragma once
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
#include <string>
#include <yaml-cpp/yaml.h>

namespace OpenXcom
{

/**
* Represents non-item game entities, like faction personell, crafts, property, etc.
* Handles all necessary methods to manipulate container.
*/
class FactionalContainer
{
private:
	std::map<std::string, int> _qty;
public:
	/// Creates an empty item container.
	FactionalContainer();
	/// Cleans up the item container.
	~FactionalContainer();
	/// Loads the item container from YAML.
	void load(const YAML::Node& node);
	/// Saves the item container to YAML.
	YAML::Node save() const;
	/// Adds an item to the container.
	void addItem(const std::string& id, int qty = 1);
	/// Removes an item from the container.
	void removeItem(const std::string& id, int qty = 1);
	/// Gets an item in the container.
	int getItem(const std::string& id) const;
	/// Gets all the items in the container.
	std::map<std::string, int>* getContents() { return &_qty; };
};

}
