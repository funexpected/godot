#! /bin/sh
VERSION=`python -c 'import version; print("%s.%s%s.%s" % (version.major, version.minor, "."+str(version.patch) if "patch" in dir(version) else "", version.status))'`
TEMPLATES="$HOME/Library/Application Support/Godot/templates/$VERSION"
if [ ! -d "$TEMPLATE" ]; then
    mkdir "$TEMPLATES"
fi
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. >/dev/null 2>&1 && pwd )"

export JAVA_HOME=$(/usr/libexec/java_home -v 1.8)

# Clean existing templates
#cd $DIR/platform/android/java/
#./gradlew cleanGodotTemplates
#cd ../../..


# Build export template(s) for platforms:

# ARM v7 (32 bit)
scons platform=android -j 8 target=release_debug android_arch=armv7 tools=no module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes

scons platform=android -j 8 target=debug android_arch=armv7 tools=no module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes

# ARM v8 (64 bit)
scons platform=android -j 8 target=release_debug android_arch=arm64v8 tools=no module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes

scons platform=android -j 8 target=debug android_arch=arm64v8 tools=no module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes

# x86 (32 bit)
#scons platform=android -j 8 target=release_debug android_arch=x86 tools=no module_firebase_enabled=no #module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes

#scons platform=android -j 8 target=debug android_arch=x86 tools=no module_firebase_enabled=no #module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes

# x86_64 (64 bit)
#scons platform=android -j 8 target=release_debug android_arch=x86_64 tools=no module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes

#scons platform=android -j 8 target=debug android_arch=x86_64 tools=no module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes


# Pack template(s) into APK(s)
cd platform/android/java/
./gradlew generateGodotTemplates
cd ../../..

# Copy the files to /executables folder
cp bin/android_debug.apk executables
cp bin/android_release.apk executables
cp bin/godot-lib.debug.aar executables
cp bin/godot-lib.release.aar executables


# Copy files to standard template location
cp bin/android_debug.apk "$TEMPLATES"
cp bin/android_release.apk "$TEMPLATES"
cp bin/android_source.zip "$TEMPLATES"

echo "$VERSION" > "$TEMPLATES/version.txt"

ROOTDIR="$(dirname "$DIR")"
echo $ROOTDIR
cp $DIR/executables/godot-lib.debug.aar $ROOTDIR/game/android/build/libs/debug 
cp $DIR/executables/godot-lib.release.aar $ROOTDIR/game/android/build/libs/release