#! /bin/sh
VERSION=`python -c 'import version; print "%s.%s.%s" % (version.major, version.minor, version.patch if "patch" in dir(version) else version.status)'`
TEMPLATES="$HOME/Library/Application Support/Godot/templates/$VERSION"
if [ ! -d "$TEMPLATE" ]; then
    mkdir "$TEMPLATES"
fi

scons p=iphone tools=no bits=64 target=release arch=arm64 module_firebase_enabled=no
scons p=iphone tools=no bits=32 target=release arch=armv7 module_firebase_enabled=no

rm -rf osx_template.app
cp -R misc/dist/osx_template.app/ osx_template.app/
mkdir osx_template.app/Contents/MacOS
cp bin/godot.osx.tools.64 osx_template.app/Contents/MacOS/godot_osx_debug.64
cp bin/godot.osx.tools.64 osx_template.app/Contents/MacOS/godot_osx_release.64
zip -X -r osx.zip osx_template.app
cp osx.zip "$TEMPLATES"
rm -rf osx_template.app

