# dpa
Distributed particle advector based on TBB and MPI.

### Requirements
- CMake 3.15+
- Git 2.24+
- GCC 9+ or Visual Studio 2019+

### (Manual) Dependencies
- Intel MKL
- Open or Microsoft MPI
- The rest are downloaded automatically through vcpkg.

## Building:
- Clone the repository.
- Run `bootstrap.[sh|bat]`. It takes approximately 20 minutes to download, build and install all dependencies.
- The binaries are then available under the `./build` folder.
