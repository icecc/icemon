#ifndef ICEMON_UTILS_H
#define ICEMON_UTILS_H

#include <QColor>

namespace utils {

// General purpose constants
const qreal Sqrt2 = 1.41421356237309504880;

// Color helpers
int getLuminance( const QColor& color );
bool isLowContrast( const QColor& color1, const QColor& color2, int treshold = 128 );
const QColor& getBetterContrastColor( const QColor& baseColor, const QColor& color1, const QColor& color2 );

}

#endif // ICEMON_UTILS_H
