dvidutils
=========

A collection of utility functions for dealing with dvid data


Installation
------------

```bash
conda install -c flyem-forge dvidutils
```

Developer Instructions
----------------------


### Linux (or Mac)

Here's how to make an ordinary Makefile-based build:

```bash

conda install -c conda-forge python=3.6 cmake xtensor-python

mkdir build
cd build

# Makefiles
cmake .. -DCMAKE_BUILD_TYPE=Debug -DXTENSOR_ENABLE_ASSERT=ON
make
make install
```

**Note:** Requires gcc >=4.9 or a recent version of clang. On CentOS, the easiesst way to get gcc-4.9 is with these commands:

```
# Install devtoolset-3
yum install -y centos-release-scl yum-utils devtoolset-3-binutils devtoolset-3-gcc devtoolset-3-gcc-c++

# Activate it (preferably in your .bashrc)
source /opt/rh/devtoolset-3/enable
```

### Xcode

On Mac, your best option for C++14 development is to use Xcode.
(On Linux, your best option is to switch to Mac.)

To use Xcode and its debugger:

```
mkdir build-for-xcode
cd build-for-xcode
cmake .. -DCMAKE_BUILD_TYPE=Debug  -DXTENSOR_ENABLE_ASSERT=ON -G Xcode
```

Xcode is finicky about which executables it likes.  Install this special build of Python:
```
conda install -c conda-forge python.app
```

Within Xcode, Opt+click the 'Run' button to edit your executable settings:

- Under "Info":
  - Select `${CONDA_PREFIX}/python.app` as the Executable (NOT `${CONDA_PREFIX}/bin/python.app`).
- Under "Arguments":
  - Add an Environment Variable for `PYTHONPATH`: `/path/to/my-dvidutils-repo/build-for-xcode/Debug`
  - Add an item to "Arguments Passed on Launch": `-m pytest --color=no /path/to/my-dvidutils-repo/tests`
