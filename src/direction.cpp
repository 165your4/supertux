//  $Id: direction.hpp 5006 2007-05-23 15:27:56Z tuxdev $
//
//  SuperTux
//  Copyright (C) 2008 Ryan Flegel <rflegel@gmail.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include "direction.hpp"

std::ostream& operator<<(std::ostream& o, const Direction& dir)
{
    switch (dir)
    {
        case AUTO:
            o << "auto";
            break;
        case LEFT:
            o << "left";
            break;
        case RIGHT:
            o << "right";
            break;
        case UP:
            o << "up";
            break;
        case DOWN:
            o << "down";
            break;
    }

    return o;
}
