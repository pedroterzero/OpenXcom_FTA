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
	FinishedCoverOperationDetailsState::FinishedCoverOperationDetailsState(CovertOperation* operation) : _operation(operation)
	{
		_screen = false;
		_results = _operation->getResults();
		// Create objects
		_window = new Window(this, 320, 200, 0, 0);
		_btnOk = new TextButton(40, 12, 16, 180);
		_btnStats = new TextButton(60, 12, 244, 180); //replacing results
		_btnResults = new TextButton(60, 12, 244, 180); //replacing stats
		_txtTitle = new Text(300, 17, 16, 8);

		_txtItem = new Text(180, 9, 16, 24);
		_lstRecoveredItems = new TextList(272, 48, 16, 32); // h=144 = 18 rows and 48 means 8 rows

		_txtReputation = new Text(180, 9, 16, 84);
		_lstReputation = new TextList(272, 24, 16, 92); //3 rows max

		_lstFunds = new TextList(290, 9, 16, 120);
		_lstScore = new TextList(290, 9, 16, 128);

		_txtSoldierStatus = new Text(180, 9, 16, 140);
		_lstSoldierStatus = new TextList(290, 16, 16, 148); //2 rows max

		_txtMessage = new Text(290, 9, 16, 168);

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

		// Set palette
		setInterface("covertOperationFinishDetails");

		add(_window, "window", "covertOperationFinishDetails");
		add(_txtTitle, "heading", "covertOperationFinishDetails");
		add(_btnOk, "button", "covertOperationFinishDetails");
		add(_btnStats, "button", "covertOperationFinishDetails");
		add(_btnResults, "button", "covertOperationFinishDetails");

		add(_txtItem, "text", "covertOperationFinishDetails");
		add(_lstRecoveredItems, "list", "covertOperationFinishDetails");
		add(_txtReputation, "text", "covertOperationFinishDetails");
		add(_lstReputation, "list", "covertOperationFinishDetails");

		add(_lstFunds, "totals", "covertOperationFinishDetails");
		add(_lstScore, "totals", "covertOperationFinishDetails");

		add(_txtSoldierStatus, "text", "covertOperationFinishDetails");
		add(_lstSoldierStatus, "list", "covertOperationFinishDetails");

		add(_txtMessage, "text", "covertOperationFinishDetails");

		centerAllSurfaces();

		// Set up objects
		setWindowBackground(_window, "covertOperationFinishDetails");
		
		_txtTitle->setBig();
		_txtTitle->setText(tr(operation->getOperationName()));


		_btnOk->setText(tr("STR_OK"));
		_btnOk->onMouseClick((ActionHandler)&FinishedCoverOperationDetailsState::btnOkClick);
		_btnOk->onKeyboardPress((ActionHandler)&FinishedCoverOperationDetailsState::btnOkClick, Options::keyOk);

		_btnStats->setText(tr("STR_OK"));
		_btnStats->onMouseClick((ActionHandler)&FinishedCoverOperationDetailsState::btnOkClick); //TODO

		_txtItem->setText(tr("STR_LIST_ITEM"));
		_lstRecoveredItems->setColumns(2, 254, 18);
		_lstRecoveredItems->setDot(true);

		_txtReputation->setText(tr("STR_REPUTATION_BONUSES"));
		_lstReputation->setColumns(2, 254, 18);
		_lstReputation->setDot(true);

		_lstFunds->setColumns(2, 254, 18);
		_lstFunds->setDot(true);
		_lstScore->setColumns(2, 254, 18);
		_lstScore->setDot(true);

		_txtSoldierStatus->setText(tr("STR_SOLDIERS_STATUS"));
		_lstSoldierStatus->setColumns(2, 254, 18);
		_lstSoldierStatus->setDot(true);

		_txtMessage->setText(tr(_results->getSpecialMessage()));



		//populate lists



		int rowItem = 0;
		std::map<std::string, int> items = _results->getItems();
		for (std::map<std::string, int>::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			auto item = tr((*i).first);
			int qty = (*i).second;
			std::ostringstream ss;
			ss << Unicode::TOK_COLOR_FLIP << qty << Unicode::TOK_COLOR_FLIP;
			_lstRecoveredItems->addRow(2, item.c_str(), ss.str().c_str());
			++rowItem;
		}

		int rowReputation = 0;
		auto factions = _results->getReputation();
		for (std::map<std::string, int>::const_iterator i = factions.begin(); i != factions.end(); ++i)
		{
			auto faction = tr((*i).first);
			int qty = (*i).second;
			std::ostringstream ss2;
			ss2 << Unicode::TOK_COLOR_FLIP << qty << Unicode::TOK_COLOR_FLIP;
			_lstReputation->addRow(2, faction.c_str(), ss2.str().c_str());
			++rowReputation;
		}

		bool hasFunds, hasScore;
		if (_results->getFunds() != 0)
		{
			std::ostringstream ss3;
			ss3 << _results->getFunds();
			_lstFunds->addRow(2, tr("STR_FUNDS_UC").c_str(), ss3.str().c_str());
			hasFunds = true;
		}
		if (_results->getScore() != 0)
		{
			std::ostringstream ss4;
			ss4 << _results->getFunds();
			_lstScore->addRow(2, tr("STR_SCORE_UC").c_str(), ss4.str().c_str());
			hasScore = true;
		}

		int rowSoldierStatus = 0;
		auto soldierStatus = _results->getSoldierDamage();
		int wounded = 0, mia = 0;
		for (std::map<std::string, int>::const_iterator i = soldierStatus.begin(); i != soldierStatus.end(); ++i)
		{
			auto soldier = tr((*i).first);
			int damage = (*i).second;
			if (damage > 0) ++wounded;
			if (damage < 0) ++mia;
			if (wounded > 0)
			{
				std::ostringstream ss5;
				ss5 << Unicode::TOK_COLOR_FLIP << wounded << Unicode::TOK_COLOR_FLIP;
				_lstSoldierStatus->addRow(2, tr("STR_XCOM_OPERATIVES_WOUNDED"), ss5.str().c_str());
				++rowSoldierStatus;
			}
			if (mia > 0)
			{
				std::ostringstream ss5;
				ss5 << Unicode::TOK_COLOR_FLIP << mia << Unicode::TOK_COLOR_FLIP;
				_lstSoldierStatus->addRow(2, tr("STR_XCOM_OPERATIVES_MISSING_IN_ACTION"), ss5.str().c_str());
				++rowSoldierStatus;
			}
		}


	}

	FinishedCoverOperationDetailsState::~FinishedCoverOperationDetailsState() {}

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
