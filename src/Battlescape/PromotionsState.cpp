/*
 * Copyright 2010-2016 OpenXcom Developers.
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
#include "PromotionsState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Basescape/SoldierInfoState.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Transfer.h"
#include "../Engine/Options.h"
#include "../Savegame/Transfer.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Promotions screen.
 * @param game Pointer to the core game.
 */
PromotionsState::PromotionsState()
{
	// Create object
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(288, 16, 16, 176);
	_txtTitle = new Text(300, 17, 10, 8);
	_txtName = new Text(114, 9, 16, 32);
	_txtRank = new Text(90, 9, 130, 32);
	_txtBase = new Text(80, 9, 220, 32);
	_lstSoldiers = new TextList(288, 128, 8, 40);

	// Set palette
	setInterface("promotions");

	add(_window, "window", "promotions");
	add(_btnOk, "button", "promotions");
	add(_txtTitle, "heading", "promotions");
	add(_txtName, "text", "promotions");
	add(_txtRank, "text", "promotions");
	add(_txtBase, "text", "promotions");
	add(_lstSoldiers, "list", "promotions");

	centerAllSurfaces();

	// Set up object
	setWindowBackground(_window, "promotions");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&PromotionsState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&PromotionsState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&PromotionsState::btnOkClick, Options::keyCancel);

	_txtTitle->setText(tr("STR_PROMOTIONS"));
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();

	_txtName->setText(tr("STR_NAME"));

	_txtRank->setText(tr("STR_NEW_RANK"));

	_txtBase->setText(tr("STR_BASE"));

	_lstSoldiers->setColumns(3, 114, 90, 84);
	_lstSoldiers->setSelectable(true);
	_lstSoldiers->setBackground(_window);
	_lstSoldiers->setMargin(8);
	_lstSoldiers->onMouseClick((ActionHandler)&PromotionsState::lstSoldiersClick);

	bool fta = _game->getMod()->isFTAGame();
	int it = 0;

	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		it = 0;
		for (std::vector<Soldier*>::iterator j = (*i)->getSoldiers()->begin(); j != (*i)->getSoldiers()->end(); ++j)
		{
			_soldierNumbersAtBase.push_back(std::make_pair((*i), it));
			it++;
			if ((*j)->isPromoted())
			{
				_lstSoldiers->addRow(3, (*j)->getName().c_str(), tr((*j)->getRankString(fta)).c_str(), (*i)->getName().c_str());
			}
		}
		// special case for soldiers, recovered from VIPs
		if (!fta)
		{ //in FtA we don't have this case basically =)
			for (std::vector<Transfer *>::iterator k = (*i)->getTransfers()->begin(); k != (*i)->getTransfers()->end(); ++k)
			{
				if ((*k)->getType() == TRANSFER_SOLDIER)
				{
					Soldier *sol = (*k)->getSoldier();
					if (sol->isPromoted())
					{
						_lstSoldiers->addRow(3, sol->getName().c_str(), tr(sol->getRankString(fta)).c_str(), (*i)->getName().c_str());
					}
				}
			}
		}
	}
}

/**
 *
 */
PromotionsState::~PromotionsState()
{
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void PromotionsState::btnOkClick(Action *)
{
	_game->popState();
}

void PromotionsState::lstSoldiersClick(Action *action)
{
	auto line = _soldierNumbersAtBase.at(_lstSoldiers->getSelectedRow());
	_game->pushState(new SoldierInfoState(line.first, line.second));
}

}
