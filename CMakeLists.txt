cmake_minimum_required(VERSION 3.28)
project(PyroRHI)

option(PYRO_RHI_ENABLE_CODE_COVERAGE "Enable code coverage reporting" OFF)

if(PYRO_RHI_ENABLE_CODE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Building with code coverage enabled (Linux / GCC/Clang)")

        # GCC/Clang coverage flags
        add_link_options(--coverage)

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0 -g")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -O0 -g")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
        message(STATUS "Building with code coverage enabled (Windows clang-cl)")

        # clang-cl coverage flags for Windows
        add_compile_options(
            /ZI              # Program database for edit & continue
            /Od              # Disable optimization for coverage accuracy
            -fsanitize-coverage=trace-pc-guard,indirect-calls
        )

        add_link_options(
            -fsanitize-coverage=trace-pc-guard,indirect-calls
        )


        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
        # Optionally define environment variable for llvm-cov
        # set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
    else()
        message(WARNING "Code coverage requested, but compiler is not supported")
    endif()
endif()


if (MSVC)
add_compile_options(
 /Zc:preprocessor   # libassert preprocessor issues
 /MP                # multithreading 
)

add_compile_options()
endif()

include(Config.cmake)

# ==== Project Config ====

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


if (PYRO_RHI_BUILD_VULKAN_VALIDATION_TESTS OR PYRO_RHI_BUILD_DX12_VALIDATION_TESTS)
enable_testing()
endif()

add_subdirectory(vendor)

add_subdirectory(PyroRHI)

if (PYRO_RHI_BUILD_VULKAN)
add_subdirectory(RHIVulkan)
endif()
if (PYRO_RHI_BUILD_DX12)
add_subdirectory(RHIDX12)
endif()
if (PYRO_RHI_BUILD_METAL)
#add_subdirectory(RHIMetal)
endif()

if (PYRO_RHI_BUILD_VULKAN_VALIDATION_TESTS OR PYRO_RHI_BUILD_DX12_VALIDATION_TESTS)
add_subdirectory(tests)
endif()


set(COMPILE_COMMANDS_FILE "${CMAKE_BINARY_DIR}/compile_commands.json")
set(DESTINATION_FILE "${CMAKE_SOURCE_DIR}/compile_commands.json")

if(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
  # Add a custom command to copy compile_commands.json after the build
  add_custom_command(OUTPUT "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}"
    DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to source directory"
  )

  # Make sure the custom command runs as part of the build. This attaches the
  # command to the ALL target, meaning it runs every time you build the project.
  add_custom_target(CopyCompileCommands ALL
    DEPENDS "${CMAKE_SOURCE_DIR}/compile_commands.json"
  )
else()
  message(STATUS "compile_commands.json not found in build directory.  Skipping copy.")
endif()
