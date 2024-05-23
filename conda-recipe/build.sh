#!/bin/bash

mkdir -p build-conda
cd build-conda

# CFLAGS="${CFLAGS} -fsanitize=address"
# CXXFLAGS="${CXXFLAGS} -fsanitize=address"
# LDFLAGS="${LDFLAGS} -fsanitize=address"

# if [[ $(uname) == "Darwin" ]]; then
#     CFLAGS="${CFLAGS} -shared-libasan"
#     CXXFLAGS="${CXXFLAGS} -shared-libasan"
#     LDFLAGS="${LDFLAGS} -shared-libasan"
# fi

## When testing with address-sanitizer, you'll need the following in your test script:
# if [[ $(uname) == "Darwin" ]]; then
#     export DYLD_INSERT_LIBRARIES=${PREFIX}/lib/clang/*/lib/darwin/libclang_rt.asan_osx_dynamic.dylib
# else
#     export LD_PRELOAD=${PREFIX}/lib/libasan.so.5
# fi

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=${PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DCMAKE_SHARED_LINKER_FLAGS="-Wl,-rpath,${PREFIX}/lib -L${PREFIX}/lib" \
    -DCMAKE_CXX_LINK_FLAGS="${LDFLAGS}" \
    -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}" \
    -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
##

make -j${CPU_COUNT} dvidutils
make install
