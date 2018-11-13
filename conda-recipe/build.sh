#!/bin/bash

mkdir -p build-conda
cd build-conda

if [[ $(uname) == 'Linux' ]]; then
	# Check which gcxx abi to use; for compatibility with libs build with gcc < 5:
	if [[ ${DO_NOT_BUILD_WITH_CXX11_ABI} == '1' ]]; then
	    CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0 ${CXXFLAGS}"
	else
	   2>&1 echo "For now, you should always build this recipe with DO_NOT_BUILD_WITH_CXX11_ABI=1"
	   2>&1 echo "Someday we'll build all of our dependencies with the CXX11 ABI, and remove this requirement."
	   2>&1 echo "(If that day is today, just remove this error from the recipe.)"
	   exit 1
	fi
fi

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=${PREFIX} \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
##

make -j${CPU_COUNT} dvidutils
make install
