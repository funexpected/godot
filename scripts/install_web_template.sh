#! /bin/sh
VERSION=`python -c 'import version; print "%s.%s%s.%s" % (version.major, version.minor, "."+str(version.patch) if "patch" in dir(version) else "", version.status)'`
TEMPLATES="$HOME/Library/Application Support/Godot/templates/$VERSION"
if [ ! -d "$TEMPLATE" ]; then
    mkdir "$TEMPLATES"
fi

scons -j 8 platform=javascript tools=no target=release
scons -j 8 platform=javascript tools=no target=release_debug

mv bin/godot.javascript.opt.zip bin/webassembly_release.zip
mv bin/godot.javascript.opt.debug.zip bin/webassembly_debug.zip

cp bin/webassembly_release.zip "$TEMPLATES"
cp bin/webassembly_debug.zip "$TEMPLATES"
