set(ROMFS_ROOT ${PX4_SOURCE_DIR}/ROMFS)

include(${ROMFS_ROOT}/CMakeLists.txt)

# Use px4fmu_common as the base ROMFS
add_subdirectory(${ROMFS_ROOT}/px4fmu_common px4fmu_common)
