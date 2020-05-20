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
#include "CovertOperationState.h"
#include <sstream>
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "NewResearchListState.h"
#include "GlobalResearchState.h"
#include "../Savegame/CovertOperation.h"
#include "../Mod/RuleCovertOperation.h"
#include "CovertOperationStartState.h"
#include "ResearchInfoState.h"
#include "TechTreeViewerState.h"
#include <algorithm>
#include "../Engine/Logger.h"

#include "../Interface/TextEdit.h"
#include "../Engine/Game.h"
#include "../Savegame/SavedGame.h"


namespace OpenXcom
{

/**
* Initializes all the elements in the CovertOperationState screen.
* @param game Pointer to the core game.
* @param base Pointer to the base to get info from.
*/
CovertOperationState::CovertOperationState(Base* base) : _base(base)
{
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnNew = new TextButton(148, 16, 8, 176);
	_btnOk = new TextButton(148, 16, 164, 176);
	_txtTitle = new Text(310, 17, 5, 8);
	_txtSoldiersAvailable = new Text(150, 9, 10, 24);
	_txtScientistsAvailable = new Text(150, 9, 160, 24);
	_txtEngineersAvailable = new Text(150, 9, 160, 34);
	//_txtSpace = new Text(150, 9, 10, 34);
	_txtOperation = new Text(150, 17, 10, 44);
	_txtChances = new Text(66, 17, 160, 44);
	_txtProgress = new Text(84, 17, 226, 44);
	_lstOperations = new TextList(302, 112, 8, 62);

	// Set palette
	setInterface("covertOperationsMenu");

	add(_window, "window", "covertOperationsMenu");
	add(_btnNew, "button", "covertOperationsMenu");
	add(_btnOk, "button", "covertOperationsMenu");
	add(_txtTitle, "text", "covertOperationsMenu");
	add(_txtSoldiersAvailable, "text", "covertOperationsMenu");
	add(_txtScientistsAvailable, "text", "covertOperationsMenu");
	add(_txtEngineersAvailable, "text", "covertOperationsMenu");
	//add(_txtSpace, "text", "covertOperationsMenu");
	add(_txtOperation, "text", "covertOperationsMenu");
	add(_txtChances, "text", "covertOperationsMenu");
	add(_txtProgress, "text", "covertOperationsMenu");
	add(_lstOperations, "list", "covertOperationsMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "covertOperationsMenu");

	_btnNew->setText(tr("STR_NEW_OPERATION"));
	_btnNew->onMouseClick((ActionHandler)&CovertOperationState::btnNewClick);
	_btnNew->onKeyboardPress((ActionHandler)&CovertOperationState::btnNewClick, Options::keyToggleQuickSearch);
	_btnNew->onKeyboardPress((ActionHandler)&CovertOperationState::onCurrentGlobalResearchClick, Options::keyGeoGlobalResearch);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CovertOperationState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CovertOperationState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_COVERT_OPERATIONS"));

	_txtOperation->setWordWrap(true);
	_txtOperation->setText(tr("STR_OPERATION_NAME"));

	_txtChances->setWordWrap(true);
	_txtChances->setText(tr("STR_OPERATION_CHANCES"));

	_txtProgress->setWordWrap(true);
	_txtProgress->setText(tr("STR_APPROX_TIME"));

	_lstOperations->setColumns(3, 150, 66, 84);
	_lstOperations->setSelectable(true);
	_lstOperations->setBackground(_window);
	_lstOperations->setMargin(2);
	_lstOperations->setWordWrap(true);
	_lstOperations->onMouseClick((ActionHandler)&CovertOperationState::onSelectOperation, SDL_BUTTON_LEFT);
	//_lstOperations->onMouseClick((ActionHandler)&CovertOperationState::onOpenTechTreeViewer, SDL_BUTTON_MIDDLE);
	//_lstOperations->onMousePress((ActionHandler)&CovertOperationState::lstResearchMousePress);
}

/**
*
*/
CovertOperationState::~CovertOperationState()
{
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void CovertOperationState::btnOkClick(Action*)
{
	_game->popState();
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void CovertOperationState::btnNewClick(Action*)
{
	_game->pushState(new CovertOperationsListState(_base));
}

/**
* Displays the selected running covert operation.
* @param action Pointer to an action.
*/
void CovertOperationState::onSelectOperation(Action*)
{
	/*const std::vector<ResearchProject*>& baseProjects(_base->getResearch());
	_game->pushState(new ResearchInfoState(_base, baseProjects[_lstOperations->getSelectedRow()]));*/
	return; //temp
}

/**
* Opens the TechTreeViewer for the corresponding topic.
* @param action Pointer to an action.
*/
void CovertOperationState::onOpenTechTreeViewer(Action*)
{
	/*const std::vector<ResearchProject*>& baseProjects(_base->getResearch());
	const RuleResearch* selectedTopic = baseProjects[_lstOperations->getSelectedRow()]->getRules();
	_game->pushState(new TechTreeViewerState(selectedTopic, 0));*/
	return; //temp
}

/**
* Handles the mouse-wheels.
* @param action Pointer to an action.
*/
void CovertOperationState::lstResearchMousePress(Action* action)
{
	if (!_lstOperations->isInsideNoScrollArea(action->getAbsoluteXMouse()))
	{
		return;
	}

	/*int change = Options::oxceResearchScrollSpeed;
	if (SDL_GetModState() & KMOD_CTRL)
		change = Options::oxceResearchScrollSpeedWithCtrl;

	if (action->getDetails()->button.button == SDL_BUTTON_WHEELUP)
	{
		change = std::min(change, _base->getAvailableScientists());
		change = std::min(change, _base->getFreeLaboratories());
		if (change > 0)
		{
			ResearchProject* selectedProject = _base->getResearch()[_lstOperations->getSelectedRow()];
			selectedProject->setAssigned(selectedProject->getAssigned() + change);
			_base->setScientists(_base->getScientists() - change);
			fillProjectList(_lstResearch->getScroll());
		}
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_WHEELDOWN)
	{
		ResearchProject* selectedProject = _base->getResearch()[_lstOperations->getSelectedRow()];
		change = std::min(change, selectedProject->getAssigned());
		if (change > 0)
		{
			selectedProject->setAssigned(selectedProject->getAssigned() - change);
			_base->setScientists(_base->getScientists() + change);
			fillProjectList(_lstResearch->getScroll());
		}
	}*/
	return; //temp
}

/**
* Opens the Current Global Research UI.
* @param action Pointer to an action.
*/
void CovertOperationState::onCurrentGlobalResearchClick(Action*)
{
	_game->pushState(new GlobalResearchState(true));
}
/**
* Updates the research list
* after going to other screens.
*/
void CovertOperationState::init()
{
	State::init();
	fillProjectList(0);

	if (Options::oxceResearchScrollSpeed > 0 || Options::oxceResearchScrollSpeedWithCtrl > 0)
	{
		// 175 +/- 20
		_lstOperations->setNoScrollArea(_lstOperations->getX() - 5, _txtScientistsAvailable->getX() + 35);
	}
	else
	{
		_lstOperations->setNoScrollArea(0, 0);
	}
}

/**
* Fills the list with Base CovertOperation. Also updates count of available lab space and available personell.
*/
void CovertOperationState::fillProjectList(size_t scrl)
{
	const std::vector<CovertOperation*>& baseOperations(_base->getCovertOperations());
	_lstOperations->clearList();
	for (std::vector<CovertOperation*>::const_iterator iter = baseOperations.begin(); iter != baseOperations.end(); ++iter)
	{
		/*std::ostringstream sstr;
		sstr << (*iter)->getOddsName();*/

		std::string wstr = tr((*iter)->getOperationName());
		_lstOperations->addRow(3, wstr.c_str(), tr((*iter)->getOddsName()).c_str(), tr((*iter)->getTimeLeftName()).c_str());
	}
	_txtSoldiersAvailable->setText(tr("STR_SOLDIERS_AVAILABLE").arg(_base->getAvailableSoldiers()));
	_txtScientistsAvailable->setText(tr("STR_SCIENTISTS_AVAILABLE").arg(_base->getAvailableScientists()));
	_txtEngineersAvailable->setText(tr("STR_ENGINEERS_AVAILABLE").arg(_base->getAvailableEngineers()));
	//Log(LOG_DEBUG) << "STR_SOLDIERS_AVAILABLE: " << _base->getAvailableSoldiers();
	//_txtSpace->setText(tr("STR_LABORATORY_SPACE_AVAILABLE").arg(_base->getFreeLaboratories()));

	if (scrl)
		_lstOperations->scrollTo(scrl);
}


/**
 * Initializes all the elements in the CovertOperations list screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
CovertOperationsListState::CovertOperationsListState(Base* base) : _base(base), _lstScroll(0)
{
	_screen = false;

	_window = new Window(this, 230, 140, 45, 30, POPUP_BOTH);
	_btnOK = new TextButton(103, 16, 164, 146);
	_txtTitle = new Text(214, 16, 53, 38);
	_lstOperations = new TextList(198, 88, 53, 54);

	// Set palette
	setInterface("covertOperationsMenu");

	add(_window, "window", "covertOperationsMenu");
	add(_btnOK, "button", "covertOperationsMenu");
	add(_txtTitle, "text", "covertOperationsMenu");
	add(_lstOperations, "list", "covertOperationsMenu");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "covertOperationsMenu");

	_btnOK->setText(tr("STR_OK"));
	_btnOK->onMouseClick((ActionHandler)&CovertOperationsListState::btnOKClick);
	_btnOK->onKeyboardPress((ActionHandler)&CovertOperationsListState::btnOKClick, Options::keyCancel);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_NEW_COVERT_OPERATIONS"));

	_lstOperations->setColumns(1, 190);
	_lstOperations->setSelectable(true);
	_lstOperations->setBackground(_window);
	_lstOperations->setMargin(8);
	_lstOperations->setAlign(ALIGN_CENTER);
	_lstOperations->onMouseClick((ActionHandler)&CovertOperationsListState::onSelectProject, SDL_BUTTON_LEFT);
	_lstOperations->onMouseClick((ActionHandler)&CovertOperationsListState::onSelectProject, SDL_BUTTON_RIGHT); //change for info
	//_lstOperations->onMouseClick((ActionHandler)&CovertOperationsListState::onSelectProject, SDL_BUTTON_MIDDLE); //change for info
}

/**
 * Initializes the screen (fills the list).
 */
void CovertOperationsListState::init()
{
	State::init();
	fillProjectList();
}

/**
 * Selects the CovertOperations to work on.
 * @param action Pointer to an action.
 */
void CovertOperationsListState::onSelectProject(Action*)
{
	_game->pushState(new CovertOperationStartState(_base, _operations[_lstOperations->getSelectedRow()]));
}


/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CovertOperationsListState::btnOKClick(Action*)
{
	_game->popState();
}

/**
 * Fills the list with possible CovertOperations.
 */
void CovertOperationsListState::fillProjectList()
{
	_operations.clear();
	_lstOperations->clearList();

	bool happy;
	for (std::vector<std::string>::const_iterator i = _game->getMod()->getCovertOperationList()->begin(); i != _game->getMod()->getCovertOperationList()->end(); ++i)
	{
		RuleCovertOperation* rule = _game->getMod()->getCovertOperation(*i);
		happy = true;
		// we dont want to do same operations twice
		const std::vector<std::string>& performedOperations = _game->getSavedGame()->getPerformedCovertOperations();
		for (std::vector<std::string>::const_iterator i = performedOperations.begin(); i != performedOperations.end(); ++i)
		{
			if (rule->getName() == (*i))
			{
				happy = false;
			}
		}
		//lets see if we have required research
		if (happy && !rule->getRequirements().empty())
		{
			if (!_game->getSavedGame()->isResearched(_game->getMod()->getResearch(rule->getRequirements())))
			{
				happy = false;
			}
		}
		//lets see if our base fits requirments
		/*if (happy)
		{
			auto providedBaseFunc = _base->getProvidedBaseFunc({});
			auto ruleReqBaseFunc = rule->getRequiresBuyBaseFunc();
			if ((~providedBaseFunc & ruleReqBaseFunc).none())
			{
				happy = false;
				break;
			}
		}*/
		//all checks passed, we can show that operation to the player
		if (happy)
		{
			_operations.push_back(rule);
			_lstOperations->addRow(1, tr((*i)).c_str());
		}
	}
	if (_lstScroll > 0)
	{
		_lstOperations->scrollTo(_lstScroll);
		_lstScroll = 0;
	}
}



}
