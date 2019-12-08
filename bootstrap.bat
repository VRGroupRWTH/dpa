

if not exist "build" mkdir build
cd build
if not exist "vcpkg" git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
if not exist "vcpkg.exe" call bootstrap-vcpkg.bat

set VCPKG_COMMAND=vcpkg install --recurse --overlay-ports="../../utility/ports"
set VCPKG_DEFAULT_TRIPLET=x64-windows
rem Add your library ports here.
%VCPKG_COMMAND% boost-mpi boost-odeint boost-ublas catch2 Eigen3 hdf5[parallel] HighFive mpi nlohmann-json tbb
cd ..

cmake -Ax64 -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --target ALL_BUILD --config Release
cd ..