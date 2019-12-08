#!/bin/bash

if [ ! -d "build" ]; then mkdir build fi
cd build
if [ ! -d "vcpkg" ]; then git clone https://github.com/Microsoft/vcpkg.git fi
cd vcpkg
if [ ! -f "vcpkg" ]; then ./bootstrap-vcpkg.sh fi

VCPKG_COMMAND=vcpkg install --recurse --overlay-ports="../../utility/ports"
VCPKG_DEFAULT_TRIPLET=x64-linux
# Add your library ports here. 
$VCPKG_COMMAND boost-mpi boost-odeint boost-ublas catch2 Eigen3 hdf5[parallel] HighFive mpi nlohmann-json tbb
cd ..

cmake -Ax64 -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --target ALL_BUILD --config Release
cd ..