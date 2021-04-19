#!/usr/bin/env bash

if [[ -z ${SWITCH_HOSTNAME} ]]; then
    echo "Env var SWITCH_HOSTNAME must be set."
    exit 1
fi

cd build
ftp -inv ${SWITCH_HOSTNAME} 5000 << EOF
cd /atmosphere/contents/0100000000000333
delete exefs.nsp
put sys-screen-capture-uploader.nsp
rename sys-screen-capture-uploader.nsp exefs.nsp
EOF
