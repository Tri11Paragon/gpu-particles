cmake_minimum_required(VERSION 3.25)
project(gpu-particles VERSION 0.0.18)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/blt-with-graphics)

include_directories(include/)
file(GLOB_RECURSE PROJECT_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

function(embed_file_as_raw_string INPUT OUTPUT NAMESPACE VAR_NAME)
    file(READ "${INPUT}" CONTENTS)
    file(WRITE "${OUTPUT}" "// Generated from ${INPUT}\n")
    file(APPEND "${OUTPUT}" "#pragma once\n\n")
    file(APPEND "${OUTPUT}" "namespace ${NAMESPACE} {\n\n")
    file(APPEND "${OUTPUT}" "static constexpr char ${VAR_NAME}_str[] = R\"(\"${CONTENTS}\")\";\n\n}")
endfunction()

function(embed_directory INPUT_DIR)
    # Get the name of the directory the user is trying to embed
    get_filename_component(parent_name "${INPUT_DIR}" NAME)
    # make sure directory uses forward slashes
    file(TO_CMAKE_PATH "${INPUT_DIR}" INPUT_DIR)
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/embedded/${parent_name}")
    # find all directories within the user folder relative to the input directory
    file(GLOB_RECURSE INPUT_FILES RELATIVE "${INPUT_DIR}" "${INPUT_DIR}/*")

    foreach(REL_PATH IN LISTS INPUT_FILES)
        set(ABS_INPUT_PATH "${INPUT_DIR}/${REL_PATH}")
        set(OUTPUT_HEADER "${OUTPUT_DIR}/${REL_PATH}.h")

        get_filename_component(OUTPUT_HEADER_DIR "${OUTPUT_HEADER}" DIRECTORY)
        file(MAKE_DIRECTORY "${OUTPUT_HEADER_DIR}")

        # Get the name, extension and relative directory of the input path + file
        get_filename_component(VAR_NAME "${REL_PATH}" NAME_WE)
        get_filename_component(VAR_EXT "${REL_PATH}" EXT)
        get_filename_component(VAR_DIR "${parent_name}/${REL_PATH}" DIRECTORY)
        set(FILE_PATH "${VAR_NAME}_${VAR_EXT}")
        # replace non-alphanum chars with "_" for the variable
        string(REGEX REPLACE "[^a-zA-Z0-9]" "_" FILE_PATH "${FILE_PATH}")
        string(REGEX REPLACE "__+" "_" FILE_PATH "${FILE_PATH}")

        # replace non-alphanum chars with : for the namespace
        string(REGEX REPLACE "[^a-zA-Z0-9]" "::" VAR_DIR ${VAR_DIR})
        string(REGEX REPLACE ":::+" "::" VAR_DIR ${VAR_DIR})

        message(STATUS "Converting file '${REL_PATH}' into variable '${VAR_NAME}' with namespace '${VAR_DIR}'")

        embed_file_as_raw_string("${ABS_INPUT_PATH}" "${OUTPUT_HEADER}" "${VAR_DIR}" "${FILE_PATH}")
    endforeach()

    include_directories("${CMAKE_BINARY_DIR}/embedded")
endfunction()

embed_directory("${CMAKE_SOURCE_DIR}/shaders")

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
