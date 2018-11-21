#! /bin/sh
VERSION=`python -c 'import version; print "%s.%s.%s" % (version.major, version.minor, version.patch if "patch" in dir(version) else version.status)'`
TEMPLATES="$HOME/Library/Application Support/Godot/templates/$VERSION"
if [ ! -d "$TEMPLATE" ]; then
    mkdir "$TEMPLATES"
fi

scons p=iphone tools=no bits=64 target=release arch=arm64 module_firebase_enabled=no
scons p=iphone tools=no bits=32 target=release arch=armv7 module_firebase_enabled=no
lipo -create bin/libgodot.iphone.opt.arm.a bin/libgodot.iphone.opt.arm64.a -output bin/libgodot.iphone.release.fat.a
scons p=iphone tools=no bits=64 target=debug arch=arm64 module_firebase_enabled=no
scons p=iphone tools=no bits=32 target=debug arch=armv7 module_firebase_enabled=no
lipo -create bin/libgodot.iphone.debug.arm.a bin/libgodot.iphone.debug.arm64.a -output bin/libgodot.iphone.debug.fat.a
rm -rf iphone*
cp -R misc/dist/ios_xcode/ iphone/
cp bin/libgodot.iphone.release.fat.a iphone/libgodot.iphone.release.fat.a
cp bin/libgodot.iphone.debug.fat.a iphone/libgodot.iphone.debug.fat.a
zip -X -r iphone.zip iphone
cp iphone.zip "$TEMPLATES"
rm -rf iphone*


