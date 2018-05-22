#! /usr/bin/env bash
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml" -name "*.h"` -L Java -o $podir/itinerary.pot
