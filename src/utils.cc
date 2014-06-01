#include "utils.h"

namespace utils {

// Color helpers

int getLuminance( const QColor& color )
{
    return color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114;
}

bool isLowContrast( const QColor& color1, const QColor& color2, int treshold )
{
    return qAbs( getLuminance( color1 ) - getLuminance( color2 ) ) < treshold;
}

const QColor& getBetterContrastColor( const QColor& baseColor, const QColor& color1, const QColor& color2 )
{
    return qAbs( getLuminance( baseColor ) - getLuminance( color1 ) ) >= qAbs( getLuminance( baseColor ) - getLuminance( color2 ) ) ? color1 : color2;
}

}
