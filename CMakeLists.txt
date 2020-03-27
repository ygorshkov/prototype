cmake_minimum_required(VERSION 3.16)
project(prototype CXX)

find_program(CONAN conan)
if (NOT CONAN)
message(FATAL_ERROR "prototype needs conan: install from https://conan.io")
endif ()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
"${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_check(VERSION 1.23 REQUIRED)
conan_cmake_run(CONANFILE conanfile.txt
BASIC_SETUP CMAKE_TARGETS
BUILD missing)

set(CMAKE_CXX_STANDARD 17)
enable_testing()

#-Wall -Werror -Wextra
add_compile_options(-Wc++17-extensions -march=native -g)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
include(CheckIPOSupported)
check_ipo_supported(RESULT LTOAvailable)
endif ()

if (LTOAvailable)
message("Link-time optimization enabled")

add_link_options(-Wl) # Magic found here: https://gcc.gnu.org/ml/gcc-help/2017-03/msg00081.html
else ()
message("Link-time optimization disabled")
endif ()

add_subdirectory(src)
#add_subdirectory(test)
add_subdirectory(lib)
#add_subdirectory(benchmarks)

if (LTOAvailable)
# Clang doesn't support a mix of LTO and non-LTO, so specify everything here
# Probably a better way to do this but GLOBAL setting didn't work.
set_property(TARGET client PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
set_property(TARGET server PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
#set_property(TARGET serialization_tests PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()
