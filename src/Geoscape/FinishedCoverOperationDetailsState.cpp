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
#include "FinishedCoverOperationDetailsState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Savegame/CovertOperation.h"
#include "../Savegame/Base.h"
#include "../Ufopaedia/Ufopaedia.h"


namespace OpenXcom
{

	/**
	 * Initializes all the elements in the FinishedCoverOperationState window.
	 * @param geoEvent Pointer to the event.
	 * @param result - comes true if sucess operation, false if it was failed.
	 */
	FinishedCoverOperationDetailsState::FinishedCoverOperationDetailsState(CovertOperation* operation) : _operation(operation), _rule(operation->getRules())
	{
		_screen = false;

		// Create objects
		// Create objects
		_window = new Window(this, 320, 200, 0, 0);
		_btnOk = new TextButton(40, 12, 16, 180);
		_btnStats = new TextButton(60, 12, 244, 180); //replace results
		_btnResults = new TextButton(60, 12, 244, 180); //replace stats
		_txtTitle = new Text(300, 17, 16, 8);
		_txtItem = new Text(180, 9, 16, 24);
		_txtQuantity = new Text(60, 9, 200, 24);
		_lstStats = new TextList(290, 80, 16, 32);
		_lstRecovery = new TextList(290, 80, 16, 32);
		_lstTotal = new TextList(290, 9, 16, 12);

		// Second page (soldier stats)
		_txtSoldier = new Text(90, 9, 16, 24); //16..106 = 90
		_txtTU = new Text(18, 9, 106, 24); //106
		_txtStamina = new Text(18, 9, 124, 24); //124
		_txtHealth = new Text(18, 9, 142, 24); //142
		_txtBravery = new Text(18, 9, 160, 24); //160
		_txtReactions = new Text(18, 9, 178, 24); //178
		_txtFiring = new Text(18, 9, 196, 24); //196
		_txtThrowing = new Text(18, 9, 214, 24); //214
		_txtMelee = new Text(18, 9, 232, 24); //232
		_txtStrength = new Text(18, 9, 250, 24); //250
		_txtPsiStrength = new Text(18, 9, 268, 24); //268
		_txtPsiSkill = new Text(18, 9, 286, 24); //286..304 = 18

		_lstSoldierStats = new TextList(288, 144, 16, 32); // 18 rows

		_txtTooltip = new Text(200, 9, 64, 180);

		// Third page (recovered items)
		_lstRecoveredItems = new TextList(272, 144, 16, 32); // 18 rows

		// Set palette
		setInterface("covertOperationFinishDetails");

		add(_window, "window", "covertOperationFinishDetails");
		add(_txtTitle, "text1", "covertOperationFinishDetails");
		add(_btnOk, "button", "covertOperationFinishDetails");
		add(_btnStats, "button", "covertOperationFinishDetails");
		add(_btnResults, "button", "covertOperationFinishDetails");

		centerAllSurfaces();

		// Set up objects

		setWindowBackground(_window, "covertOperationFinishDetails");
		
		_txtTitle->setAlign(ALIGN_CENTER);
		_txtTitle->setBig();
		_txtTitle->setWordWrap(true);
		_txtTitle->setText(tr(operation->getOperationName()));


		_btnOk->setText(tr("STR_OK"));
		_btnOk->onMouseClick((ActionHandler)&FinishedCoverOperationDetailsState::btnOkClick);
		_btnOk->onKeyboardPress((ActionHandler)&FinishedCoverOperationDetailsState::btnOkClick, Options::keyOk);

	}

	/**
	 *
	 */
	FinishedCoverOperationDetailsState::~FinishedCoverOperationDetailsState()
	{
		// Empty by design
	}

	//methods to generate UI

	std::string FinishedCoverOperationDetailsState::makeSoldierString(int stat)
	{
		if (stat == 0) return "";

		std::ostringstream ss;
		ss << Unicode::TOK_COLOR_FLIP << '+' << stat << Unicode::TOK_COLOR_FLIP;
		return ss.str();
	}

	/**
	 * Closes the window and shows a pedia article if needed.
	 * @param action Pointer to an action.
	 */
	void FinishedCoverOperationDetailsState::btnOkClick(Action*)
	{
		_game->popState();
	}
	/**
	 * Shows Finished Covert Opration details.
	 * @param action Pointer to an action.
	 */
	void FinishedCoverOperationDetailsState::btnDetailsClick(Action* action)
	{
	}

	
	

}
