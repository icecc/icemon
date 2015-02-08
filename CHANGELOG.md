## 3.0.0 (2015-02-08)

Features:

- Star View got a fresh look, improved Detailed Host View (https://github.com/icecc/icemon/pull/17)
- Added simple man page (https://github.com/icecc/icemon/pull/9)
- Starting a change log

Bugfixes:

- Fix RPATH issues (https://github.com/icecc/icemon/pull/18)
- Lots of other small bug fixes in the Icemon views

Removals:

- Dropped the "Pool View", which was never fully implemented

Internal Changes:

- Ported icemon to Qt5
- No longer depends on kdelibs
- Better separation of model/view throughout the source code
    - Enables possibility to write QtQuick-based views now
