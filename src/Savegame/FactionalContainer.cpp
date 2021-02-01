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
#include "FactionalContainer.h"

namespace OpenXcom
{

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
