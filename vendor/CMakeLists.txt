# Common

# Default path for PyroCommon if user does not provide one
if(NOT DEFINED PYRO_COMMON_DIR)
    set(PYRO_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PyroCommon" CACHE PATH "Path to PyroCommon")
    set(PYRO_COMMON_BUILD_TESTS OFF CACHE BOOL "Build tests for PyroCommon" FORCE)
    message(STATUS "PYRO_COMMON_DIR not set, defaulting to ${PYRO_COMMON_DIR}")
endif()

# Try to find an installed version of PyroCommon first
find_package(PyroCommon CONFIG QUIET)

# If not found, fall back to building it from source
if(NOT PyroCommon_FOUND)
    if(NOT EXISTS "${PYRO_COMMON_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "PyroCommon not found at ${PYRO_COMMON_DIR}")
    endif()

    message(STATUS "Using PyroCommon from source at ${PYRO_COMMON_DIR}")
    set(PYRO_COMMON_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/PyroCommon") # out-of-tree build
    add_subdirectory(${PYRO_COMMON_DIR} ${PYRO_COMMON_BINARY_DIR})
endif()


if(PYRO_RHI_BUILD_VULKAN)
    # vulkan
    find_package(Vulkan REQUIRED)

    # volk
    set(VOLK_PULL_IN_VULKAN ON)
    set(VOLK_INSTALL OFF)
    set(VOLK_HEADERS_ONLY OFF)
    add_subdirectory(volk)

    # vma
    set(VMA_ENABLE_INSTALL ON)
    set(VMA_BUILD_DOCUMENTATION OFF)
    set(VMA_BUILD_SAMPLES OFF)
    add_subdirectory(VulkanMemoryAllocator)
endif()
if(PYRO_RHI_BUILD_DX12)
    # d3d12 utils
    set(DXHEADERS_INSTALL ON)
    add_subdirectory(DirectX-Headers)

    # d3d12ma
    set(D3D12MA_BUILD_SAMPLE OFF)
    add_subdirectory(D3D12MemoryAllocator)
endif()