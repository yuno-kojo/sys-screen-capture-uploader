#!/usr/bin/env bash

set -e

rm -rf release
mkdir release
cd release
mkdir -p config/sys-screen-capture-uploader
mkdir -p atmosphere/contents/0100000000000333/flags
cp ../config.ini config/sys-screen-capture-uploader/config.ini
cp ../toolbox.json atmosphere/contents/0100000000000333/toolbox.json
cp ../build/sys-screen-capture-uploader.nsp atmosphere/contents/0100000000000333/exefs.nsp
touch atmosphere/contents/0100000000000333/flags/boot2.flag
zip -r sys-screen-capture-uploader *
