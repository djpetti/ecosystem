#!/bin/bash

# Work from the script directory
cd "$(dirname "$0")"

./download_externals.sh
GYP=externals/downloaded/gyp/gyp
NINJA=externals/downloaded/ninja/ninja

GYP_GENERATORS=ninja ${GYP} ecosystem.gyp -I ecosystem.gypi \
    --toplevel-dir=`pwd` --depth=.. --no-circular-check
${NINJA} -C out/Default/ all
