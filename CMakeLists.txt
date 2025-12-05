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

if (PYRO_RHI_BUILD_DX12)
include(DX12AgilitySDK.cmake)
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
add_coverage_instrumentation(RHIValidationTestDX12)
endif()
if (PYRO_RHI_BUILD_VULKAN_VALIDATION_TESTS)
add_coverage_instrumentation(RHIVulkan)
add_coverage_instrumentation(RHIValidationTestVulkan)
endif()

add_coverage_instrumentation(RHIBaseTest)
endif()

