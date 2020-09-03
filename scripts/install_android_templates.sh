#! /bin/sh
VERSION=`python -c 'import version; print "%s.%s%s.%s" % (version.major, version.minor, "."+str(version.patch) if "patch" in dir(version) else "", version.status)'`
TEMPLATES="$HOME/Library/Application Support/Godot/templates/$VERSION"
if [ ! -d "$TEMPLATE" ]; then
    mkdir "$TEMPLATES"
fi

# Clean existing templates
./platform/android/java/gradlew cleanGodotTemplates


# Build export template(s) for platforms(s):

# ARM v7 (32 bit)
#scons platform=android target=release android_arch=armv7
#scons platform=android target=debug android_arch=armv7

# ARM v8 (64 bit)
#scons platform=android target=release android_arch=arm64v8
scons platform=android -j 4 target=debug android_arch=arm64v8 tools=no module_firebase_enabled=no module_bullet_enabled=no module_websocket_enabled=no game_center=no disable_3d=yes debug_symbols=yes separate_debug_symbols=yes

# x86 (32 bit)
#scons platform=android target=release android_arch=x86
#scons platform=android target=debug android_arch=x86

# x86_64 (64 bit)
#scons platform=android target=release android_arch=x86_64
#scons platform=android target=debug android_arch=x86_64


# Pack template(s) into APK(s)
./platform/android/java/gradlew generateGodotTemplates


# Copy APKs to a standard template location
cp bin/android_debug.apk "$TEMPLATES"
#cp bin/android_release.apk "$TEMPLATES"

echo "$VERSION" > "$TEMPLATES/version.txt"