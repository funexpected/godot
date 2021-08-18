#! /bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. >/dev/null 2>&1 && pwd )"
cd $DIR
rm -rf iphone*
cp -R misc/dist/ios_xcode/ iphone/
scons p=iphone -j 4 tools=no bits=64 target=release_debug arch=arm64 module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no debug_symbols=yes separate_debug_symbols=yes || exit 1
cp bin/libgodot.iphone.opt.debug.arm64.a iphone/libgodot.iphone.release.fat.a

#scons p=iphone -j 4 tools=no bits=64 target=debug arch=arm64 module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no debug_symbols=yes separate_debug_symbols=yes || exit 1
cp bin/libgodot.iphone.opt.debug.arm64.a iphone/libgodot.iphone.debug.fat.a
zip -X -r iphone.zip iphone
cp iphone.zip bin/ios.templates.zip
rm -rf iphone*



