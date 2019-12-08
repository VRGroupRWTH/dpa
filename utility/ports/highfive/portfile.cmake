include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO BlueBrain/HighFive
    REF v2.1.1
    PATCHES doc.patch
    SHA512 3ae7189e19725c23f6169cf3037d0b4ebe92ebab64551db3235d1a70ffca26c54a3533b4af419789ab9cdd27a16582c091f2ee58591223c4255bdc0783e1f65c
    HEAD_REF master
)

if(${VCPKG_LIBRARY_LINKAGE} MATCHES "static")
    set(HDF5_USE_STATIC_LIBRARIES ON)
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DHIGHFIVE_UNIT_TESTS=OFF
        -DHIGHFIVE_EXAMPLES=OFF
        -DUSE_BOOST=OFF
        -DHDF5_USE_STATIC_LIBRARIES=${HDF5_USE_STATIC_LIBRARIES}
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH share/HighFive/CMake)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug)
if(NOT (NOT VCPKG_CMAKE_SYSTEM_NAME OR VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore") AND NOT VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/share/HighFive)
endif()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/highfive RENAME copyright)
