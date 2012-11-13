#ifndef ICEMON_VERSION_H
#define ICEMON_VERSION_H

#include "config-icemon.h"

#include <qglobal.h>

namespace Icemon {
namespace Version {

const char * const appName = QT_TRANSLATE_NOOP("appName", "Icecream Monitor" );
const char * const appShortName = "icemon";
const char * const version = ICEMON_VERSION_STRING;
const char * const homePage = "http://github.com/icecc/icemon";
const char * const description = QT_TRANSLATE_NOOP( "description", "Icecream monitor for Qt" );
const char * const copyright = QT_TRANSLATE_NOOP( "copyright", "(c) 2003, 2004, 2011 The Icecream developers" );

} // Version
} // Icemon

#endif // ICEMON_VERSION_H
