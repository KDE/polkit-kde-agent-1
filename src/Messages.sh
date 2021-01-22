#!/usr/bin/env bash

$XGETTEXT $(find -name \*.cpp -o -name \*.h -o \*.qml) $podir/polkit-kde-authentication-agent-1.pot
