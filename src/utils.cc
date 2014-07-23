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

#include "utils.h"

int Utils::luminance( const QColor& color )
{
    return color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114;
}

bool Utils::isLowContrast( const QColor& color1, const QColor& color2, int treshold )
{
    return qAbs( luminance( color1 ) - luminance( color2 ) ) < treshold;
}

QColor Utils::betterContrastColor( const QColor& baseColor, const QColor& color1, const QColor& color2 )
{
    return qAbs( luminance( baseColor ) - luminance( color1 ) ) >= qAbs( luminance( baseColor ) - luminance( color2 ) ) ? color1 : color2;
}

QColor Utils::textColor(const QColor& baseColor)
{
    return betterContrastColor( baseColor, Qt::black, Qt::white );
}
