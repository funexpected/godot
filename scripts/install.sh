#! /bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. >/dev/null 2>&1 && pwd )"
cd $DIR
./scripts/install_editor.sh
./scripts/install_ios_templates.sh