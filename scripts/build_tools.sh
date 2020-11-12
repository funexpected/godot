#! /bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. >/dev/null 2>&1 && pwd )"
cd $DIR
scons -j8 platform=osx
cp bin/godot.osx.tools.64 executables/godot
scons -j8 platform=server tool=yes target=release_debug
cp bin/godot_server.osx.opt.tools.64 executables/godot.headless


