#!/bin/sh

if [ ! -d "build" ]; then mkdir build; fi
cd build
if [ ! -d "vcpkg" ]; then git clone https://github.com/Microsoft/vcpkg.git; fi
cd vcpkg
if [ ! -f "vcpkg" ]; then ./bootstrap-vcpkg.sh; fi

if [ ! -d "custom-triplets" ]; then mkdir custom-triplets; fi
cp ./triplets/x64-linux.cmake ./custom-triplets/x64-linux-static.cmake
cp ../../utility/vcpkg/x64-linux-dynamic.cmake ./custom-triplets/x64-linux.cmake

VCPKG_DEFAULT_TRIPLET=x64-linux
vcpkg install boost-mpi boost-odeint boost-ublas catch2 Eigen3 hdf5[parallel] intel-mkl mpi nlohmann-json
vcpkg install tbb --overlay-triples=custom-triplets
cd ..

cmake -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --target ALL_BUILD --config Release
cd ..
