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
#include "CovertOperationEquipmentState.h"
#include "CovertOperationStartState.h"
#include "CovertOperationSoldiersState.h"
#include <climits>
#include <sstream>
#include <algorithm>
#include <locale>
#include "../Engine/CrossPlatform.h"
#include "../Engine/Screen.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/Timer.h"
#include "../Engine/Collections.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/ComboBox.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "../Interface/TextList.h"
#include "../Mod/Armor.h"
#include "../Savegame/Base.h"
#include "../Savegame/Craft.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Mod/RuleCraft.h"
#include "../Savegame/ItemContainer.h"
#include "../Mod/RuleItemCategory.h"
#include "../Mod/RuleItem.h"
#include "../Savegame/Vehicle.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/CovertOperation.h"
#include "../Menu/ErrorMessageState.h"
#include "../Battlescape/CannotReequipState.h"
#include "../Battlescape/DebriefingState.h"
#include "../Battlescape/InventoryState.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Mod/RuleInterface.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
* Initializes all the elements in the CovertOperation Equipment screen.
* @param base Pointer to the base to get info from.
* @param operation Pointer to starting (not comitted) covert operation.
*/
CovertOperationEquipmentState::CovertOperationEquipmentState(Base* base, CovertOperationStartState* operation) : _lstScroll(0), _sel(0), _base(base), _operation(operation), _totalItems(0), _ammoColor(0), _reload(true)
{
	_rule = operation->getRule();
	bool hasSoldiers = false; // operation->getSoldiers().size() > 0; //#FINNIKTODO

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnQuickSearch = new TextEdit(this, 48, 9, 264, 12);
	_btnOk = new TextButton((hasSoldiers) ? 30 : 140, 16, (hasSoldiers) ? 274 : 164, 176);
	_btnInventory = new TextButton(102, 16, 164, 176);
	_txtTitle = new Text(300, 17, 16, 7);
	_txtItem = new Text(144, 9, 16, 32);
	_txtStores = new Text(150, 9, 160, 32);
	_txtAvailable = new Text(110, 9, 16, 24);
	_txtCrew = new Text(110, 9, 125, 24);
	_txtChances = new Text(110, 9, 200, 24);
	_lstEquipment = new TextList(288, 128, 8, 40);
	_cbxFilterBy = new ComboBox(this, 140, 16, 16, 176, true);

	// Set palette
	setInterface("operationEquipment");

	_ammoColor = _game->getMod()->getInterface("operationEquipment")->getElement("ammoColor")->color;

	add(_window, "window", "operationEquipment");
	add(_btnQuickSearch, "button", "operationEquipment");
	add(_btnOk, "button", "operationEquipment");
	add(_btnInventory, "button", "operationEquipment");
	add(_txtTitle, "text", "operationEquipment");
	add(_txtItem, "text", "operationEquipment");
	add(_txtStores, "text", "operationEquipment");
	add(_txtAvailable, "text", "operationEquipment");
	add(_txtCrew, "text", "operationEquipment");
	add(_txtChances, "text", "operationEquipment");
	add(_lstEquipment, "list", "operationEquipment");
	add(_cbxFilterBy, "button", "operationEquipment");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "operationEquipment");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CovertOperationEquipmentState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CovertOperationEquipmentState::btnOkClick, Options::keyCancel);
	_btnOk->onKeyboardPress((ActionHandler)&CovertOperationEquipmentState::btnClearClick, Options::keyRemoveEquipmentFromCraft);


	_btnInventory->setText(tr("STR_INVENTORY"));
	_btnInventory->onMouseClick((ActionHandler)&CovertOperationEquipmentState::btnInventoryClick);
	_btnInventory->setVisible(hasSoldiers);
	_btnInventory->onKeyboardPress((ActionHandler)&CovertOperationEquipmentState::btnInventoryClick, Options::keyBattleInventory);

	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_EQUIPMENT_FOR_CRAFT").arg(tr(_rule->getName())));

	_txtItem->setText(tr("STR_ITEM"));

	_txtStores->setText(tr("STR_STORES"));

	std::ostringstream ss;
	double itemsSize = _operation->getItems()->getTotalSize(_game->getMod());
	ss << itemsSize << "/" << _rule->getItemSpaceLimit(); 
	_txtAvailable->setText(tr("STR_SPACE_USED").arg(ss.str()));
	_txtAvailable->setVisible(_rule->getItemSpaceLimit() >= 0);

	std::ostringstream ss3;
	ss3 << tr("STR_SOLDIERS_UC") << ">" << Unicode::TOK_COLOR_FLIP << operation->getSoldiers().size();
	_txtCrew->setText(ss3.str());

	bool mod = _game->getSavedGame()->getDebugMode();
	_txtChances->setText(tr("STR_OPERATION_CHANCES_US").arg(tr(_operation->getOperationOddsString(mod))));

	// populate sort options
	_categoryStrings.push_back("STR_ALL");
	_categoryStrings.push_back("STR_EQUIPPED");
	bool hasUnassigned = false;
	const std::vector<std::string>& items = _game->getMod()->getItemsList();
	for (std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		RuleItem* rule = _game->getMod()->getItem(*i);
		auto isVehicle = rule->getVehicleUnit();
		int cQty = operation->getItems()->getItem(*i);

		if ((!isVehicle || rule->isInventoryItem()) && rule->canBeEquippedToCraftInventory() &&
			_game->getSavedGame()->isResearched(rule->getRequirements()) &&
			(_base->getStorageItems()->getItem(*i) > 0 || cQty > 0))
		{
			if (rule->getCategories().empty())
			{
				hasUnassigned = true;
			}
			else
			{
				for (std::vector<std::string>::const_iterator j = rule->getCategories().begin(); j != rule->getCategories().end(); ++j)
				{
					_usedCategoryStrings[(*j)] = true;
				}
			}
		}
	}
	const std::vector<std::string>& itemCategories = _game->getMod()->getItemCategoriesList();
	for (std::vector<std::string>::const_iterator i = itemCategories.begin(); i != itemCategories.end(); ++i)
	{
		if (_usedCategoryStrings[(*i)])
		{
			if (!_game->getMod()->getItemCategory((*i))->isHidden())
			{
				_categoryStrings.push_back((*i));
			}
		}
	}
	if (hasUnassigned && !itemCategories.empty())
	{
		_categoryStrings.push_back("STR_UNASSIGNED");
	}
	_categoryStrings.push_back("STR_NOT_EQUIPPED");

	_cbxFilterBy->setOptions(_categoryStrings, true);
	_cbxFilterBy->setSelected(0);
	_cbxFilterBy->onChange((ActionHandler)&CovertOperationEquipmentState::cbxFilterByChange);

	_lstEquipment->setArrowColumn(203, ARROW_HORIZONTAL);
	_lstEquipment->setColumns(3, 156, 83, 41);
	_lstEquipment->setSelectable(true);
	_lstEquipment->setBackground(_window);
	_lstEquipment->setMargin(8);
	_lstEquipment->onLeftArrowPress((ActionHandler)&CovertOperationEquipmentState::lstEquipmentLeftArrowPress);
	_lstEquipment->onLeftArrowRelease((ActionHandler)&CovertOperationEquipmentState::lstEquipmentLeftArrowRelease);
	_lstEquipment->onLeftArrowClick((ActionHandler)&CovertOperationEquipmentState::lstEquipmentLeftArrowClick);
	_lstEquipment->onRightArrowPress((ActionHandler)&CovertOperationEquipmentState::lstEquipmentRightArrowPress);
	_lstEquipment->onRightArrowRelease((ActionHandler)&CovertOperationEquipmentState::lstEquipmentRightArrowRelease);
	_lstEquipment->onRightArrowClick((ActionHandler)&CovertOperationEquipmentState::lstEquipmentRightArrowClick);
	_lstEquipment->onMousePress((ActionHandler)&CovertOperationEquipmentState::lstEquipmentMousePress);

	_btnQuickSearch->setText(""); // redraw
	_btnQuickSearch->onEnter((ActionHandler)&CovertOperationEquipmentState::btnQuickSearchApply);
	_btnQuickSearch->setVisible(false);

	_btnOk->onKeyboardRelease((ActionHandler)&CovertOperationEquipmentState::btnQuickSearchToggle, Options::keyToggleQuickSearch);

	_timerLeft = new Timer(250);
	_timerLeft->onTimer((StateHandler)&CovertOperationEquipmentState::moveLeft);
	_timerRight = new Timer(250);
	_timerRight->onTimer((StateHandler)&CovertOperationEquipmentState::moveRight);
}

/**
*
*/
CovertOperationEquipmentState::~CovertOperationEquipmentState()
{
	delete _timerLeft;
	delete _timerRight;
}

/**
* Filters the equipment list by the selected criterion
* @param action Pointer to an action.
*/
void CovertOperationEquipmentState::cbxFilterByChange(Action* action)
{
	initList();
}

/**
* Resets the savegame when coming back from the inventory.
*/
void CovertOperationEquipmentState::init()
{
	State::init();

	_game->getSavedGame()->setBattleGame(0);

	// don't reload after closing error popups
	if (_reload)
	{
		initList();
		std::ostringstream ss;
		double itemsSize = _operation->getItems()->getTotalSize(_game->getMod());
		ss << itemsSize << "/" << _rule->getItemSpaceLimit();
		_txtAvailable->setText(tr("STR_SPACE_USED").arg(ss.str()));
		bool mod = _game->getSavedGame()->getDebugMode();
		_txtChances->setText(tr("STR_OPERATION_CHANCES_US").arg(tr(_operation->getOperationOddsString(mod))));
	}
	_reload = true;
}

/**
* Quick search toggle.
* @param action Pointer to an action.
*/
void CovertOperationEquipmentState::btnQuickSearchToggle(Action* action)
{
	if (_btnQuickSearch->getVisible())
	{
		_btnQuickSearch->setText("");
		_btnQuickSearch->setVisible(false);
		btnQuickSearchApply(action);
	}
	else
	{
		_btnQuickSearch->setVisible(true);
		_btnQuickSearch->setFocus(true);
	}
}

/**
* Quick search.
* @param action Pointer to an action.
*/
void CovertOperationEquipmentState::btnQuickSearchApply(Action*)
{
	initList();
}

/**
* Shows the equipment in a list filtered by selected criterion.
*/
void CovertOperationEquipmentState::initList()
{
	std::string searchString = _btnQuickSearch->getText();
	Unicode::upperCase(searchString);

	size_t selIdx = _cbxFilterBy->getSelected();
	if (selIdx == (size_t)-1)
	{
		return;
	}
	const std::string selectedCategory = _categoryStrings[selIdx];
	bool categoryFilterEnabled = (selectedCategory != "STR_ALL");
	bool categoryUnassigned = (selectedCategory == "STR_UNASSIGNED");
	bool categoryEquipped = (selectedCategory == "STR_EQUIPPED");
	bool shareAmmoCategories = _game->getMod()->getShareAmmoCategories();
	bool categoryNotEquipped = (selectedCategory == "STR_NOT_EQUIPPED");

	// reset
	_totalItems = 0;
	_items.clear();
	_lstEquipment->clearList();

	int row = 0;
	const std::vector<std::string>& items = _game->getMod()->getItemsList();
	for (std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		RuleItem* rule = _game->getMod()->getItem(*i);

		//auto isVehicle = rule->getVehicleUnit();
		int cQty = 0;
		cQty = _operation->getItems()->getItem(*i);
		_totalItems += cQty;

		if ((rule->isInventoryItem()) && rule->canBeEquippedToCraftInventory() &&
			(_base->getStorageItems()->getItem(*i) > 0 || cQty > 0))
		{
			// check research requirements
			if (!_game->getSavedGame()->isResearched(rule->getRequirements()))
			{
				continue;
			}

			// filter by category
			if (categoryFilterEnabled)
			{
				if (categoryUnassigned)
				{
					if (!rule->getCategories().empty())
					{
						continue;
					}
				}
				else if (categoryEquipped)
				{
					if (!(cQty > 0))
					{
						continue;
					}
				}
				else if (categoryNotEquipped)
				{
					if (cQty > 0)
					{
						continue;
					}
				}
				else
				{
					bool isOK = rule->belongsToCategory(selectedCategory);
					if (shareAmmoCategories && !isOK && rule->getBattleType() == BT_FIREARM)
					{
						for (auto* ammoRule : *rule->getPrimaryCompatibleAmmo())
						{
							if (_base->getStorageItems()->getItem(ammoRule) > 0 || _operation->getItems()->getItem(ammoRule) > 0)
							{
								if (ammoRule->isInventoryItem() && ammoRule->canBeEquippedToCraftInventory() && _game->getSavedGame()->isResearched(ammoRule->getRequirements()))
								{
									isOK = ammoRule->belongsToCategory(selectedCategory);
									if (isOK) break;
								}
							}
						}
					}
					if (!isOK) continue;
				}
			}

			// quick search
			if (!searchString.empty())
			{
				std::string projectName = tr((*i));
				Unicode::upperCase(projectName);
				if (projectName.find(searchString) == std::string::npos)
				{
					continue;
				}
			}

			_items.push_back(*i);
			std::ostringstream ss, ss2;
			if (_game->getSavedGame()->getMonthsPassed() > -1)
			{
				ss << _base->getStorageItems()->getItem(*i);
			}
			else
			{
				ss << "-";
			}
			ss2 << cQty;

			std::string s = tr(*i);
			if (rule->getBattleType() == BT_AMMO)
			{
				s.insert(0, "  ");
			}
			_lstEquipment->addRow(3, s.c_str(), ss.str().c_str(), ss2.str().c_str());

			Uint8 color;
			if (cQty == 0)
			{
				if (rule->getBattleType() == BT_AMMO)
				{
					color = _ammoColor;
				}
				else
				{
					color = _lstEquipment->getColor();
				}
			}
			else
			{
				color = _lstEquipment->getSecondaryColor();
			}
			_lstEquipment->setRowColor(row, color);

			++row;
		}
	}

	_lstEquipment->draw();
	if (_lstScroll > 0)
	{
		_lstEquipment->scrollTo(_lstScroll);
		_lstScroll = 0;
	}
}

/**
	* Runs the arrow timers.
	*/
void CovertOperationEquipmentState::think()
{
	State::think();

	_timerLeft->think(this, 0);
	_timerRight->think(this, 0);
}


/**
	* Returns to the previous screen.
	* @param action Pointer to an action.
	*/
void CovertOperationEquipmentState::btnOkClick(Action*)
{
	_game->popState();
}

/**
	* Starts moving the item to the base.
	* @param action Pointer to an action.
	*/
void CovertOperationEquipmentState::lstEquipmentLeftArrowPress(Action* action)
{
	_sel = _lstEquipment->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT && !_timerLeft->isRunning())
		_timerLeft->start();
}

/**
	* Stops moving the item to the base.
	* @param action Pointer to an action.
	*/
void CovertOperationEquipmentState::lstEquipmentLeftArrowRelease(Action* action)
{
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		_timerLeft->stop();
	}
}

/**
	* Moves all the items to the base on right-click.
	* @param action Pointer to an action.
	*/
void CovertOperationEquipmentState::lstEquipmentLeftArrowClick(Action* action)
{
	if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		int operationItemsCount = _operation->getItems()->getItem(_items[_sel]);
		moveLeftByValue(operationItemsCount);
	}
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		moveLeftByValue(1);
		_timerRight->setInterval(250);
		_timerLeft->setInterval(250);
	}
}

/**
	* Starts moving the item to the craft.
	* @param action Pointer to an action.
	*/
void CovertOperationEquipmentState::lstEquipmentRightArrowPress(Action* action)
{
	_sel = _lstEquipment->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT && !_timerRight->isRunning())
	{
		_timerRight->start();
	}
}

/**
	* Stops moving the item to the craft.
	* @param action Pointer to an action.
	*/
void CovertOperationEquipmentState::lstEquipmentRightArrowRelease(Action* action)
{
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		_timerRight->stop();
	}
}

/**
	* Moves all the items (as much as possible) to the craft on right-click.
	* @param action Pointer to an action.
	*/
void CovertOperationEquipmentState::lstEquipmentRightArrowClick(Action* action)
{
	if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		int baseItemsCount = _base->getStorageItems()->getItem(_items[_sel]);
		moveRightByValue(baseItemsCount, true);
	}

	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		moveRightByValue(1);
		_timerRight->setInterval(250);
		_timerLeft->setInterval(250);
	}
}

/**
	* Handles the mouse-wheels on the arrow-buttons.
	* @param action Pointer to an action.
	*/
void CovertOperationEquipmentState::lstEquipmentMousePress(Action* action)
{
	_sel = _lstEquipment->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_WHEELUP)
	{
		_timerRight->stop();
		_timerLeft->stop();
		if (action->getAbsoluteXMouse() >= _lstEquipment->getArrowsLeftEdge() &&
			action->getAbsoluteXMouse() <= _lstEquipment->getArrowsRightEdge())
		{
			moveRightByValue(Options::changeValueByMouseWheel);
		}
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_WHEELDOWN)
	{
		_timerRight->stop();
		_timerLeft->stop();
		if (action->getAbsoluteXMouse() >= _lstEquipment->getArrowsLeftEdge() &&
			action->getAbsoluteXMouse() <= _lstEquipment->getArrowsRightEdge())
		{
			moveLeftByValue(Options::changeValueByMouseWheel);
		}
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_MIDDLE)
	{
		_lstScroll = _lstEquipment->getScroll();
		RuleItem* rule = _game->getMod()->getItem(_items[_sel]);
		std::string articleId = rule->getType();
		Ufopaedia::openArticle(_game, articleId);
	}
}

/**
* Updates the displayed quantities of the
* selected item on the list.
*/
void CovertOperationEquipmentState::updateQuantity()
{
	RuleItem* item = _game->getMod()->getItem(_items[_sel], true);
	int cQty = 0;
	cQty = _operation->getItems()->getItem(_items[_sel]);
	std::ostringstream ss, ss2;
	ss << _base->getStorageItems()->getItem(_items[_sel]);
	ss2 << cQty;

	Uint8 color;
	if (cQty == 0)
	{
		if (item->getBattleType() == BT_AMMO)
		{
			color = _ammoColor;
		}
		else
		{
			color = _lstEquipment->getColor();
		}
	}
	else
	{
		color = _lstEquipment->getSecondaryColor();
	}
	_lstEquipment->setRowColor(_sel, color);
	_lstEquipment->setCellText(_sel, 1, ss.str());
	_lstEquipment->setCellText(_sel, 2, ss2.str());

	std::ostringstream sse;
	double itemsSize = _operation->getItems()->getTotalSize(_game->getMod());
	sse << itemsSize << "/" << _rule->getItemSpaceLimit();
	_txtAvailable->setText(tr("STR_SPACE_USED").arg(sse.str()));
	bool mod = _game->getSavedGame()->getDebugMode();
	_txtChances->setText(tr("STR_OPERATION_CHANCES_US").arg(tr(_operation->getOperationOddsString(mod))));
}

/**
* Moves the selected item to the base.
*/
void CovertOperationEquipmentState::moveLeft()
{
	_timerLeft->setInterval(50);
	_timerRight->setInterval(50);
	moveLeftByValue(1);
}

/**
* Moves the given number of items (selected) to the base.
* @param change Item difference.
*/
void CovertOperationEquipmentState::moveLeftByValue(int change)
{
	RuleItem* item = _game->getMod()->getItem(_items[_sel], true);
	int operationItemsCount = _operation->getItems()->getItem(_items[_sel]);
	if (change <= 0 || operationItemsCount <= 0) return;
	change = std::min(operationItemsCount, change);
	_operation->getItems()->removeItem(_items[_sel], change);
	_totalItems -= item->getSize() * change;
	_base->getStorageItems()->addItem(_items[_sel], change);
	updateQuantity();
}

/**
* Moves the selected item to the craft.
*/
void CovertOperationEquipmentState::moveRight()
{
	_timerLeft->setInterval(50);
	_timerRight->setInterval(50);
	moveRightByValue(1);
}

/**
* Moves the given number of items (selected) to the craft.
* @param change Item difference.
* @param suppressErrors Suppress error messages? (usually used to handle right-click)
*/
void CovertOperationEquipmentState::moveRightByValue(int change, bool suppressErrors)
{
	RuleItem* item = _game->getMod()->getItem(_items[_sel], true);
	int baseItemsCount = _base->getStorageItems()->getItem(_items[_sel]);
	if (0 >= change || 0 >= baseItemsCount) return;
	change = std::min(baseItemsCount, change);
	double sumAdd = _totalItems + item->getSize() * change;
	if (_rule->getItemSpaceLimit() >= 0 && sumAdd > _rule->getItemSpaceLimit())
	{
		// get free space and divide into item size
		change = (_rule->getItemSpaceLimit() - _totalItems) / item->getSize();
		// check double right-click
		if (!suppressErrors || (suppressErrors && baseItemsCount>0 && change == 0))
		{
			_timerRight->stop();
			LocalizedText msg (tr("STR_OVER_ITEM_SIZE_LIMIT", _rule->getItemSpaceLimit()));
			_game->pushState
			(
				new ErrorMessageState
				(
					msg,
					_palette,
					_game->getMod()->getInterface("operationEquipment")->getElement("errorMessage")->color,
					_game->getMod()->getInterface("operationEquipment")->getBackgroundImage(),
					_game->getMod()->getInterface("operationEquipment")->getElement("errorPalette")->color
				)
			);
			_reload = false;
			return;
		}
		else
		{
			change = (_rule->getItemSpaceLimit() - _totalItems) / item->getSize();
		}
	}
	_operation->getItems()->addItem(_items[_sel], change);
	_totalItems += item->getSize() * change;
	if (_game->getSavedGame()->getMonthsPassed() > -1)
	{
		_base->getStorageItems()->removeItem(_items[_sel], change);
	}
	updateQuantity();
}

/**
* Empties the contents of the craft, moving all of the items back to the base.
*/
void CovertOperationEquipmentState::btnClearClick(Action*)
{
	for (_sel = 0; _sel != _items.size(); ++_sel)
	{
		moveLeftByValue(INT_MAX);
	}
}

/**
* Displays the inventory screen for the soldiers
* inside the craft.
* @param action Pointer to an action.
*/
void CovertOperationEquipmentState::btnInventoryClick(Action*)
{
	/*if (_operation->getSoldiers().size() > 0)
	{
		CovertOperation* preOp = new CovertOperation(_operation->getRule(), _base);
		for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
		{
			bool matched = false;
			auto iter = std::find(std::begin(_operation->getSoldiers()), std::end(_operation->getSoldiers()), (*i));
			if (iter != std::end(_operation->getSoldiers())) matched = true;
			if (matched) (*i)->setCovertOperation(preOp);
		}
		SavedBattleGame* bgame = new SavedBattleGame(_game->getMod(), _game->getLanguage());
		_game->getSavedGame()->setBattleGame(bgame);

		if (_game->isCtrlPressed() && _game->isAltPressed())
		{
			_game->getSavedGame()->setDisableSoldierEquipment(true);
		}
		BattlescapeGenerator bgen = BattlescapeGenerator(_game);
		bgen.runInventory(0, preOp);

		_game->getScreen()->clear();
		_game->pushState(new InventoryState(false, 0, _base, true));

		for (std::vector<Soldier*>::iterator i = _base->getSoldiers()->begin(); i != _base->getSoldiers()->end(); ++i)
		{
			bool matched = false;
			auto iter = std::find(std::begin(_operation->getSoldiers()), std::end(_operation->getSoldiers()), (*i));
			if (iter != std::end(_operation->getSoldiers())) matched = true;
			if (matched) (*i)->setCovertOperation(0);
		}
		delete preOp;
	}
	else
	{
		return;
	}*/
	return;
}


}
