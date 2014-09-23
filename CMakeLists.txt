project(valfendi)
cmake_minimum_required(VERSION 2.8)

add_executable(valfendi main.cpp validate.cpp)

install(TARGETS valfendi DESTINATION bin)
include(TestBigEndian)
include(CheckIncludeFiles)

test_big_endian(BIGENDIAN)

set(VALFENDI_MAJOR_VERSION 0)
set(VALFENDI_MINOR_VERSION 2)
set(VALFENDI_PATCH_VERSION 0pre4)
set(VALFENDI_VERSION ${VALFENDI_MAJOR_VERSION}.${VALFENDI_MINOR_VERSION}.${VALFENDI_PATCH_VERSION})

include_directories(${PROJECT_BINARY_DIR})
configure_file (config.h.in config.h)

set(CPACK_PACKAGE_VERSION_MAJOR ${VALFENDI_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${VALFENDI_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${VALFENDI_PATCH_VERSION})
set(CPACK_SOURCE_IGNORE_FILES /\\\\.git;.*~)
include(CPack)
