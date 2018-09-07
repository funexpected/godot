#! /bin/sh
VERSION=`python -c 'import version; print "%s.%s%s.%s" % (version.major, version.minor, "."+str(version.patch) if "patch" in dir(version) else "", version.status)'`
TEMPLATES="$HOME/Library/Application Support/Godot/templates/$VERSION"
if [ ! -d "$TEMPLATE" ]; then
    mkdir "$TEMPLATES"
fi

rm -rf iphone*
cp -R misc/dist/ios_xcode/ iphone/

# this is min required build, but for now try size-optimized build
scons p=iphone -j 4 tools=no bits=64 target=release arch=arm64 module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes debug_symbols=yes separate_debug_symbols=yes || exit 1
scons p=iphone -j 4 tools=no bits=64 target=debug arch=arm64 module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes debug_symbols=yes separate_debug_symbols=yes || exit 1

# this is optimized build, but it is too slow on export
#scons p=iphone target=release arch=arm64 optimize=size use_lto=yes tools=no module_bullet_enabled=no module_firebase_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes 
#scons p=iphone target=debug arch=arm64 optimize=size use_lto=yes tools=no module_bullet_enabled=no module_firebase_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes 

# no need to compile armv7 as far as we target only arm64 devices
#scons p=iphone tools=no bits=32 target=release arch=armv7 module_firebase_enabled=no module_websocket_enabled=no game_center=no
#scons p=iphone tools=no bits=32 target=debug arch=armv7 module_firebase_enabled=no module_websocket_enabled=no game_center=no
#lipo -create bin/libgodot.iphone.opt.arm.a bin/libgodot.iphone.opt.arm64.a -output bin/libgodot.iphone.release.fat.a
#lipo -create bin/libgodot.iphone.debug.arm.a bin/libgodot.iphone.debug.arm64.a -output bin/libgodot.iphone.debug.fat.a
#cp bin/libgodot.iphone.release.fat.a iphone/libgodot.iphone.release.fat.a
#cp bin/libgodot.iphone.debug.fat.a iphone/libgodot.iphone.debug.fat.a

cp bin/libgodot.iphone.opt.arm64.a iphone/libgodot.iphone.release.fat.a
cp bin/libgodot.iphone.debug.arm64.a iphone/libgodot.iphone.debug.fat.a

zip -X -r iphone.zip iphone
cp iphone.zip "$TEMPLATES"
rm -rf iphone*


