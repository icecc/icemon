## 3.3.0 (?)

Features:
- Display node protocol version and features in detailed host view
- Several more icecream flavors added
- Add --port option to specify the scheduler port

Bugfixes:
- Remote hardcoded background color (#39)
- Fix data loss when switching views
- Avoid a possible QSocketNotifier crash (#47)
- Improve column widths in views

Internal Changes:
- Require extra-cmake-modules for building
- Fix finding icecc pkgconfig file

## 3.2.0 (2018-12-10)

Bugfixes:

- Fix broken scheduler discovery with newer Icecream (#40)

Internal Changes:

- Add some badges to README.md
- Coding style improvements

## 3.1.0 (2017-04-07)

Features:

- Summary view: Multiple improvements (#23)
    - Displays average time for each submitted jobs
    - Added display of average build time for finished jobs
- Added scheduler hostname option (#27)

Bugfixes:

- Fixed summary view stateWidget color not updated correctly (#23)
- ListView: Sorted file sizes correctly (643abfbbdeed806aa5a08f0c1cfcdaf7ba79d748)
- Fixed filtering in detailed host view (#26)

Internal Changes:

- Lots of cleanups, more strict compiler flags, etc.

## 3.0.1 (2016-02-06)

Bugfixes:

- Added work-around for build for icecc.a using old CXXABI (#24)
- Fixed build with Qt 5.5
- Improved how docbook2man is searched for (PR #21)

Internal Changes:

- Added Doxygen support to CMake
- Modernized CMake code (FindIcecream.cmake, etc.)
- Modernized source code to use C++11 features (override, nullptr, auto)

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
