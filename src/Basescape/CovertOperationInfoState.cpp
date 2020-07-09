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
#include "CovertOperationInfoState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCovertOperation.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/CovertOperation.h"
#include <algorithm>
#include <unordered_set>

namespace OpenXcom
{
	/**
	 * Initializes all the elements on the UI.
	 */
	CovertOperationInfoState::CovertOperationInfoState(CovertOperation * operation) : _operation(operation)
	{

		// Create objects
		_window = new Window(this, 320, 200, 0, 0);
		_txtTitle = new Text(304, 17, 8, 7);
		_btnTerminate = new TextButton(148, 16, 8, 176);
		_btnOk = new TextButton(148, 16, 164, 176);

		// Set palette
		setInterface("covertOperationInfoState");

		add(_window, "window", "covertOperationInfoState");
		add(_txtTitle, "text", "covertOperationInfoState");
		add(_btnOk, "button", "covertOperationInfoState");
		add(_btnTerminate, "button", "covertOperationInfoState");

		centerAllSurfaces();

		// Set up objects
		setWindowBackground(_window, "covertOperationInfoState");

		_txtTitle->setBig();
		_txtTitle->setAlign(ALIGN_CENTER);
		_txtTitle->setText("Anton, load operation name to me, please =)"); //TODO

		_btnTerminate->setText(tr("STR_TERMINATE_OPERATION"));
		_btnTerminate->onMouseClick((ActionHandler)&CovertOperationInfoState::btnTerminateClick);

		_btnOk->setText(tr("STR_OK"));
		_btnOk->onMouseClick((ActionHandler)&CovertOperationInfoState::btnOkClick);
		_btnOk->onKeyboardPress((ActionHandler)&CovertOperationInfoState::btnOkClick, Options::keyCancel);

		//TODO more elements here
		
	}

	/**
	 *
	 */
	CovertOperationInfoState::~CovertOperationInfoState()
	{
	}

	/**
	* Returns to the previous screen.
	* @param action Pointer to an action.
	*/
	void CovertOperationInfoState::btnOkClick(Action*)
	{
		_game->popState();
	}

	/**
	* Opens the Select Topic screen.
	* @param action Pointer to an action.
	*/
	void CovertOperationInfoState::btnTerminateClick(Action*)
	{
		_game->pushState(new CovertOperationConfirmTerminateState(_operation));
	}

	//TODO more state methods here, please


	/**
	 * Initializes all the elements on the UI.
	 */
	CovertOperationConfirmTerminateState::CovertOperationConfirmTerminateState(CovertOperation* operation) : _operation(operation)
	{

		// Create objects
		_window = new Window(this, 320, 200, 0, 0);
		_txtText = new Text(304, 17, 8, 7);
		_btnOk = new TextButton(148, 16, 8, 176);
		_btnCancel = new TextButton(148, 16, 164, 176);

		// Set palette
		setInterface("covertOperationConfirmTerminateState");

		add(_window, "window", "covertOperationConfirmTerminateState");
		add(_txtText, "text", "covertOperationConfirmTerminateState");
		add(_btnOk, "button", "covertOperationConfirmTerminateState");

		centerAllSurfaces();

		// Set up objects
		setWindowBackground(_window, "covertOperationConfirmTerminateState");

		_txtText->setBig();
		_txtText->setAlign(ALIGN_CENTER);
		_txtText->setVerticalAlign(ALIGN_MIDDLE);
		_txtText->setText(tr("STR_TERMINATE_OPERATION_TEXT"));

		_btnOk->setText(tr("STR_OK"));
		_btnOk->onMouseClick((ActionHandler)&CovertOperationConfirmTerminateState::btnOkClick);

		_btnCancel->setText(tr("STR_CANCEL_UC"));
		_btnCancel->onMouseClick((ActionHandler)&CovertOperationConfirmTerminateState::btnCancelClick);
		_btnCancel->onKeyboardPress((ActionHandler)&CovertOperationConfirmTerminateState::btnCancelClick, Options::keyCancel);

	}

	CovertOperationConfirmTerminateState::~CovertOperationConfirmTerminateState()
	{
	}

	/**
	* Returns to the previous screen.
	* @param action Pointer to an action.
	*/
	void CovertOperationConfirmTerminateState::btnOkClick(Action*)
	{
		return; //TODO add operation termination here pls
	}

	/**
	* Opens the Select Topic screen.
	* @param action Pointer to an action.
	*/
	void CovertOperationConfirmTerminateState::btnCancelClick(Action*)
	{
		_game->popState();
	}

}
