cmake_minimum_required(VERSION 3.20)
project(Wins2 VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(WINS2_BUILD_TESTS "Build Wins2 unit tests" ON)
add_library(wins2_core
 src/core/memory.cpp
 src/core/iop_memory.cpp
 src/disc/ul_loader.cpp
 src/disc/iso9660.cpp
 src/disc/cdvd_controller.cpp
 src/core/save_state.cpp
 src/cpu/ee/r5900.cpp
 src/cpu/iop/r3000a.cpp
 src/frontend/user_paths.cpp
)
target_include_directories(wins2_core PUBLIC src)
if(MSVC)
 target_compile_options(wins2_core PRIVATE /W4 /permissive-)
else()
 target_compile_options(wins2_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_executable(wins2 src/main.cpp)
target_link_libraries(wins2 PRIVATE wins2_core)
if(WINS2_BUILD_TESTS)
 enable_testing()
 add_executable(wins2_tests tests/test_memory.cpp)
 target_link_libraries(wins2_tests PRIVATE wins2_core)
 add_test(NAME memory_tests COMMAND wins2_tests)
 add_executable(wins2_iop_memory_tests tests/test_iop_memory.cpp)
 target_link_libraries(wins2_iop_memory_tests PRIVATE wins2_core)
 add_test(NAME iop_memory_tests COMMAND wins2_iop_memory_tests)
 add_executable(wins2_r5900_tests tests/test_r5900.cpp)
 target_link_libraries(wins2_r5900_tests PRIVATE wins2_core)
 add_test(NAME r5900_tests COMMAND wins2_r5900_tests)
 add_executable(wins2_r3000a_tests tests/test_r3000a.cpp)
 target_link_libraries(wins2_r3000a_tests PRIVATE wins2_core)
 add_test(NAME r3000a_tests COMMAND wins2_r3000a_tests)
 add_executable(wins2_storage_tests tests/test_storage.cpp)
 target_link_libraries(wins2_storage_tests PRIVATE wins2_core)
 add_test(NAME storage_tests COMMAND wins2_storage_tests)
 add_executable(wins2_ul_tests tests/test_ul_loader.cpp)
 target_link_libraries(wins2_ul_tests PRIVATE wins2_core)
 add_test(NAME ul_loader_tests COMMAND wins2_ul_tests)
 add_executable(wins2_iso_tests tests/test_iso9660.cpp)
 target_link_libraries(wins2_iso_tests PRIVATE wins2_core)
 add_test(NAME iso9660_tests COMMAND wins2_iso_tests)
endif()
