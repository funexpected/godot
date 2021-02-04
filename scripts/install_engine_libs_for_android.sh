#! /bin/sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. >/dev/null 2>&1 && pwd )"

ROOTDIR="$(dirname "$DIR")"

echo $ROOTDIR

cp $DIR/executables/godot-lib.debug.aar $ROOTDIR/game/android/build/libs/debug 
cp $DIR/executables/godot-lib.release.aar $ROOTDIR/game/android/build/libs/release