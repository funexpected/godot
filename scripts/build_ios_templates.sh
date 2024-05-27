#! /bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. >/dev/null 2>&1 && pwd )"
cd $DIR

# TODO: download MoltenVK.xcframework to misc/dist/ios_xcode/MoltenVK.xcframework
# python3 ./scripts/download_godot_templates.py

rm -rf ios*
cp -R misc/dist/ios_xcode/ ios/
scons p=ios -j 8  target=template_debug  arch=arm64 module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no debug_symbols=yes separate_debug_symbols=yes || exit 1
cp bin/libgodot.ios.template_debug.arm64.a ios/libgodot.ios.debug.xcframework/ios-arm64/libgodot.a

scons p=ios -j 4 tools=no bits=64 target=template_release arch=arm64 module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no debug_symbols=yes separate_debug_symbols=yes || exit 1
cp bin/libgodot.ios.opt.template_release.arm64.a ios/libgodot.ios.release.xcframework/ios-arm64/libgodot.a

cd ios
zip -X -r ../ios.zip *
cd ..

cp ios.zip bin/ios.templates.zip
rm -rf ios*



