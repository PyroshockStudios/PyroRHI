cmake_minimum_required(VERSION 3.28)
project(PyroRHI)

include(CodeCoverage.cmake)

if (MSVC)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		add_compile_options(-Wno-unused-command-line-argument)
	else()
		add_compile_options(
		 /Zc:preprocessor   # libassert preprocessor issues
		 /MP                # multithreading 
		)
	endif()
endif()

include(Config.cmake)

# ==== Project Config ====

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


if (PYRO_RHI_BUILD_TESTS)
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

if (PYRO_RHI_BUILD_TESTS)
add_subdirectory(tests)

if (PYRO_RHI_BUILD_DX12_VALIDATION_TESTS)
add_coverage_instrumentation(RHIDX12)
endif()
if (PYRO_RHI_BUILD_VULKAN_VALIDATION_TESTS)
add_coverage_instrumentation(RHIVulkan)
endif()

add_coverage_instrumentation(RHIBaseTest)
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
