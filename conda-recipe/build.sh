#!/bin/bash

mkdir -p build-conda
cd build-conda

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=${PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
##

make -j${CPU_COUNT}
make install
