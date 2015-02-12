/*
    This file is part of Icecream.

    Copyright (c) 2014 Kevin Funk <kfunk@kde.org>

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

#ifndef ICEMON_VERSION_H
#define ICEMON_VERSION_H

#include "config-icemon.h"

#include <qglobal.h>

namespace Icemon {
namespace Version {
const char appName[] = QT_TRANSLATE_NOOP("appName", "Icecream Monitor");
const char appShortName[] = "icemon";
const char version[] = ICEMON_VERSION_STRING;
const char homePage[] = "http://github.com/icecc/icemon";
const char description[] = QT_TRANSLATE_NOOP("description", "Icecream monitor for Qt");
const char copyright[] = QT_TRANSLATE_NOOP("copyright", "(c) 2003, 2004, 2011 The Icecream developers");
} // Version
} // Icemon

#endif // ICEMON_VERSION_H
