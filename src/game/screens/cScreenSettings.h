/*
* Copyright 2009, 2010, The Pink Petal Development Team.
* The Pink Petal Devloment Team are defined as the game's coders
* who meet on http://pinkpetal.org     // old site: http://pinkpetal .co.cc
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "interface/cInterfaceWindowXML.h"

class cScreenSettings : public cInterfaceWindowXML
{
public:
    void set_ids() override;

    cScreenSettings();
    void init(bool back) override;
    void process() override { };

    void update_settings();

private:
    int ok_id = -1;
    int revert_id = -1;

    // Folders
    int characters_id = -1;
    int saves_id = -1;
    int defaultimages_id = -1;
    int items_id = -1;
    int preferdefault_id = -1;

    int theme_id = -1;
    int fullscreen_id = -1;
    int width_id = -1;
    int height_id = -1;
};