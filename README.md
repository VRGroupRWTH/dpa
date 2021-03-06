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
- Run as `mpiexec -n [NUMBER_OF_NODES] -ppn 1 ./dpa [PATH_TO_CONFIG_FILE]`. See the utility directory for example config files.

## Notes:
- The input must consist of a 1D float spacing attribute and a 4D XYZV float dataset specified in the config file.
- The output is generated as one HDF5 file per rank, each consisting of three entries per round; two 1D float arrays for the vertices/colors and a 1D uint32/uint64 array for the indices.
- The HDF5 files are accompanied by one XDMF file per rank.
- When recording curves, if particles_per_round * iterations > maximum uint32_t, uint64_t indices are used.