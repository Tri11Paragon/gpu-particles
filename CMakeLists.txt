cmake_minimum_required(VERSION 3.25)
project(gpu-particles VERSION 0.0.10)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/blt-with-graphics)

include_directories(include/)
file(GLOB_RECURSE PROJECT_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(gpu-particles ${PROJECT_BUILD_FILES})

target_compile_options(gpu-particles PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
target_link_options(gpu-particles PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)

target_link_libraries(gpu-particles PRIVATE BLT_WITH_GRAPHICS)

if (${ENABLE_ADDRSAN} MATCHES ON)
    target_compile_options(gpu-particles PRIVATE -fsanitize=address)
    target_link_options(gpu-particles PRIVATE -fsanitize=address)
endif ()

if (${ENABLE_UBSAN} MATCHES ON)
    target_compile_options(gpu-particles PRIVATE -fsanitize=undefined)
    target_link_options(gpu-particles PRIVATE -fsanitize=undefined)
endif ()

if (${ENABLE_TSAN} MATCHES ON)
    target_compile_options(gpu-particles PRIVATE -fsanitize=thread)
    target_link_options(gpu-particles PRIVATE -fsanitize=thread)
endif ()
