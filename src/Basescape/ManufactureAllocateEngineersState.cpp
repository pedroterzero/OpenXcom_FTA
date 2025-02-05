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
#include "ManufactureAllocateEngineersState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleSoldier.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Production.h"
#include "../Mod/RuleManufacture.h"
#include "../Basescape/ManufactureInfoStateFtA.h"
#include "../Basescape/ManufactureProductDetailsState.h"
#include "SoldierInfoStateFtA.h"
#include <algorithm>

namespace OpenXcom
{

/**
* Initializes all the elements in the CovertOperation Soldiers screen.
* @param base Pointer to the base to get info from.
* @param operation Pointer to starting (not committed) covert operation.
*/
ManufactureAllocateEngineersState::ManufactureAllocateEngineersState(Base* base, ManufactureInfoStateFtA* planningProject)
	: _base(base), _planningProject(planningProject), _otherCraftColor(0), _origSoldierOrder(*_base->getSoldiers()), _dynGetter(NULL)
{
	_freeSpace = planningProject->getWorkspace();

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(148, 16, 164, 176);
	_btnInfo = new TextButton(42, 16, 270, 8);
	_txtTitle = new Text(300, 17, 16, 7);
	_txtName = new Text(114, 9, 16, 32);
	_txtAssignment = new Text(84, 9, 122, 32);
	_txtFreeSpace = new Text(150, 9, 16, 24);
	_cbxSortBy = new ComboBox(this, 148, 16, 8, 176, true);
	_lstEngineers = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("manufactureAllocateEngineers");

	add(_window, "window", "manufactureAllocateEngineers");
	add(_btnOk, "button", "manufactureAllocateEngineers");
	add(_btnInfo, "button2", "manufactureAllocateEngineers");
	add(_txtTitle, "text", "manufactureAllocateEngineers");
	add(_txtName, "text", "manufactureAllocateEngineers");
	add(_txtAssignment, "text", "manufactureAllocateEngineers");
	add(_txtFreeSpace, "text", "manufactureAllocateEngineers");
	add(_lstEngineers, "list", "manufactureAllocateEngineers");
	add(_cbxSortBy, "button", "manufactureAllocateEngineers");

	_otherCraftColor = _game->getMod()->getInterface("manufactureAllocateEngineers")->getElement("otherCraft")->color;

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "manufactureAllocateEngineers");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&ManufactureAllocateEngineersState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&ManufactureAllocateEngineersState::btnOkClick, Options::keyCancel);

	_btnInfo->setText(tr("STR_INFO"));
	_btnInfo->onMouseClick((ActionHandler)&ManufactureAllocateEngineersState::btnInfoClick);

	_txtTitle->setBig();
	_txtTitle->setText(tr(planningProject->getManufactureRules()->getName()));
	_txtTitle->setWordWrap(true);
	_txtTitle->setVerticalAlign(ALIGN_MIDDLE);

	_txtName->setText(tr("STR_NAME_UC"));

	_txtAssignment->setText(tr("STR_ASSIGNMENT"));

	// populate sort options
	std::vector<std::string> sortOptions;
	sortOptions.push_back(tr("STR_ORIGINAL_ORDER"));
	_sortFunctors.push_back(NULL);

#define PUSH_IN(strId, functor)       \
sortOptions.push_back(tr(strId)); \
_sortFunctors.push_back(new SortFunctor(_game, functor));

	PUSH_IN("STR_ID", idStat);
	PUSH_IN("STR_NAME_UC", nameStat);

	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::weaponry), weaponryStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::explosives), explosivesStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::microelectronics), microelectronicsStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::metallurgy), metallurgyStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::processing), processingStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::efficiency), efficiencyStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::diligence), diligenceStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::hacking), hackingStat);
	PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::construction), constructionStat);
	if (_game->getSavedGame()->isResearched(_game->getMod()->getAlienTechUnlockResearch()))
	{
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::alienTech), alienTechStat);
		PUSH_IN(OpenXcom::UnitStats::getStatString(&UnitStats::reverseEngineering), reverseEngineeringStat);
	}

#undef PUSH_IN

	_cbxSortBy->setOptions(sortOptions);
	_cbxSortBy->setSelected(0);
	_cbxSortBy->onChange((ActionHandler)&ManufactureAllocateEngineersState::cbxSortByChange);
	_cbxSortBy->setText(tr("STR_SORT_BY"));

	_lstEngineers->setColumns(2, 106, 174);
	_lstEngineers->setAlign(ALIGN_RIGHT, 3);
	_lstEngineers->setSelectable(true);
	_lstEngineers->setBackground(_window);
	_lstEngineers->setMargin(8);
	_lstEngineers->onMouseClick((ActionHandler)&ManufactureAllocateEngineersState::lstEngineersClick, 0);
}

/**
* cleans up dynamic state
*/
ManufactureAllocateEngineersState::~ManufactureAllocateEngineersState()
{
	for (std::vector<SortFunctor*>::iterator it = _sortFunctors.begin();
		it != _sortFunctors.end(); ++it)
	{
		delete (*it);
	}
}

/**
* Sorts the soldiers list by the selected criterion
* @param action Pointer to an action.
*/
void ManufactureAllocateEngineersState::cbxSortByChange(Action*)
{
	bool ctrlPressed = _game->isCtrlPressed();
	size_t selIdx = _cbxSortBy->getSelected();
	if (selIdx == (size_t)-1)
	{
		return;
	}

	SortFunctor* compFunc = _sortFunctors[selIdx];
	_dynGetter = NULL;
	if (compFunc)
	{
		if (selIdx != 2)
		{
			_dynGetter = compFunc->getGetter();
		}

		// if CTRL is pressed, we only want to show the dynamic column, without actual sorting
		if (!ctrlPressed)
		{
			if (selIdx == 2)
			{
				std::stable_sort(_base->getSoldiers()->begin(), _base->getSoldiers()->end(),
					[](const Soldier* a, const Soldier* b)
					{
						return Unicode::naturalCompare(a->getName(), b->getName());
					});
			}
			else
			{
				std::stable_sort(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), *compFunc);
			}
			if (_game->isShiftPressed())
			{
				std::reverse(_base->getSoldiers()->begin(), _base->getSoldiers()->end());
			}
		}
	}
	else
	{
		// restore original ordering, ignoring (of course) those
		// soldiers that have been sacked since this state started
		for (std::vector<Soldier*>::const_iterator it = _origSoldierOrder.begin();
			it != _origSoldierOrder.end(); ++it)
		{
			std::vector<Soldier*>::iterator soldierIt =
				std::find(_base->getSoldiers()->begin(), _base->getSoldiers()->end(), *it);
			if (soldierIt != _base->getSoldiers()->end())
			{
				Soldier* s = *soldierIt;
				_base->getSoldiers()->erase(soldierIt);
				_base->getSoldiers()->insert(_base->getSoldiers()->end(), s);
			}
		}
	}

	size_t originalScrollPos = _lstEngineers->getScroll();
	initList(originalScrollPos);
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void ManufactureAllocateEngineersState::btnOkClick(Action*)
{
	_planningProject->setAssignedEngineers();
	_game->popState();
}

void ManufactureAllocateEngineersState::btnInfoClick(Action* action)
{
	_game->pushState(new ManufactureProductDetailsState(_base, _planningProject->getManufactureRules()));
}

/**
* Shows the soldiers in a list at specified offset/scroll.
*/
void ManufactureAllocateEngineersState::initList(size_t scrl)
{
	int row = 0;
	_lstEngineers->clearList();
	_filteredListOfEngineers.clear();
	_engineerNumbers.clear();
	int i = 0;

	for (auto& soldier : *_base->getSoldiers())
	{
		if (soldier->getRoleRank(ROLE_ENGINEER) > 0)
		{
			_filteredListOfEngineers.push_back(soldier);
			_engineerNumbers.push_back(i);
		}
		i++;
	}

	if (_dynGetter != NULL)
	{
		_lstEngineers->setColumns(3, 106, 158, 16);
	}
	else
	{
		_lstEngineers->setColumns(2, 106, 174);
	}

	auto recovery = _base->getSumRecoveryPerDay();
	bool isBusy = false, isFree = false;
	for (std::vector<Soldier*>::iterator s = _filteredListOfEngineers.begin(); s != _filteredListOfEngineers.end(); ++s)
	{
		std::string duty = (*s)->getCurrentDuty(_game->getLanguage(), recovery, isBusy, isFree, WORK);
		if (_dynGetter != NULL)
		{
			// call corresponding getter
			int dynStat = (*_dynGetter)(_game, *s);
			std::ostringstream ss;
			ss << dynStat;
			_lstEngineers->addRow(3, (*s)->getName(true, 19).c_str(), duty.c_str(), ss.str().c_str());
		}
		else
		{
			_lstEngineers->addRow(2, (*s)->getName(true, 19).c_str(), duty.c_str());
		}

		Uint8 color = _lstEngineers->getColor();
		bool matched = false;
		auto engineers = _planningProject->getEngineers();
		auto iter = std::find(std::begin(engineers), std::end(engineers), (*s));
		if (iter != std::end(engineers))
		{
			matched = true;
		}

		if (matched)
		{
			color = _lstEngineers->getSecondaryColor();
			_lstEngineers->setCellText(row, 1, tr("STR_ASSIGNED_UC"));
		}
		else if (isBusy || !isFree)
		{
			color = _otherCraftColor;
		}

		_lstEngineers->setRowColor(row, color);
		row++;
	}
	if (scrl)
		_lstEngineers->scrollTo(scrl);
	_lstEngineers->draw();

	_txtFreeSpace->setText(tr("STR_WORKSHOP_SPACE_AVAILABLE").arg(_freeSpace));
}

/**
* Shows the soldiers in a list.
*/
void ManufactureAllocateEngineersState::init()
{
	State::init();
	_base->prepareSoldierStatsWithBonuses(); // refresh stats for sorting
	initList(0);
}

/**
* Shows the selected soldier's info.
* @param action Pointer to an action.
*/
void ManufactureAllocateEngineersState::lstEngineersClick(Action* action)
{
	double mx = action->getAbsoluteXMouse();
	if (mx >= _lstEngineers->getArrowsLeftEdge() && mx < _lstEngineers->getArrowsRightEdge())
	{
		return;
	}
	int row = _lstEngineers->getSelectedRow();
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		Soldier* s = _base->getSoldiers()->at(_engineerNumbers.at(row));
		Uint8 color = _lstEngineers->getColor();
		bool isBusy = false, isFree = false, matched = false;
		std::string duty = s->getCurrentDuty(_game->getLanguage(), _base->getSumRecoveryPerDay(), isBusy, isFree, WORK);
		auto engineers = _planningProject->getEngineers();
		auto iter = std::find(std::begin(engineers), std::end(engineers), s);
		if (iter != std::end(engineers))
		{
			matched = true;
		}
		if (matched)
		{
			_planningProject->removeEngineer(s);
			if (s->getProductionProject())
			{
				if (s->getProductionProject()->getRules() == _planningProject->getManufactureRules())
				{
					s->setProductionProject(0);
					color = _lstEngineers->getColor();
					_lstEngineers->setCellText(row, 1, tr("STR_NONE_UC"));
					_freeSpace++;
				}
				else
				{
					color = _otherCraftColor;
					_lstEngineers->setCellText(row, 1, duty);
				}
			}
			else
			{
				_lstEngineers->setCellText(row, 1, duty); //#FINNIKCHECK
				_freeSpace++;
				if (isBusy || !isFree || s->getCraft())
				{
					color = _otherCraftColor;
				}
			}
		}
		else if (s->hasFullHealth() && !isBusy)
		{
			bool noProject = s->getProductionProject() == 0;
			if (noProject && _freeSpace <= 0)
			{
				_game->pushState(new ErrorMessageState(tr("STR_NOT_ENOUGH_WORKSPACE"),
					_palette,
					_game->getMod()->getInterface("soldierInfo")->getElement("errorMessage")->color,
					"BACK01.SCR",
					_game->getMod()->getInterface("soldierInfo")->getElement("errorPalette")->color));
			}
			else
			{
				_lstEngineers->setCellText(row, 1, tr("STR_ASSIGNED_UC"));
				color = _lstEngineers->getSecondaryColor();
				_planningProject->addEngineer(s);
				if (noProject)
				{
					_freeSpace--;
				}
			}
		}

		_lstEngineers->setRowColor(row, color);
		_txtFreeSpace->setText(tr("STR_WORKSHOP_SPACE_AVAILABLE").arg(_freeSpace));
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
		_game->pushState(new SoldierInfoStateFtA(_base, _engineerNumbers.at(row)));
	}
}

}
