
#! /bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. >/dev/null 2>&1 && pwd )"
cd $DIR
scons -j8 platform=osx tools=yes module_mono_enabled=yes mono_glue=no
./bin/godot.osx.tools.64.mono --generate-mono-glue modules/mono/glue
scons -j8 platform=osx tools=yes module_mono_enabled=yes mono_glue=yes
cp bin/godot.osx.tools.64.mono executables/godot.mono
cp -r bin/GodotSharp executables/GodotSharp
