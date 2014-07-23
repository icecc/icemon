/*
    This file is part of Icecream.

    Copyright (c) 2014 Robert Płóciennik <rob.plociennik@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef ICEMON_UTILS_H
#define ICEMON_UTILS_H

#include <QColor>

namespace Utils {

int luminance( const QColor& color );
bool isLowContrast( const QColor& color1, const QColor& color2, int treshold = 128 );
const QColor betterContrastColor( const QColor& baseColor, const QColor& color1, const QColor& color2 );

}

#endif // ICEMON_UTILS_H
