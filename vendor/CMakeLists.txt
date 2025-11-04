# Common

# Default path for PyroCommon if user does not provide one
if(NOT TARGET PyroCommon)
    find_package(PyroCommon CONFIG QUIET)
    if(NOT PyroCommon_FOUND)
        set(PYRO_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PyroCommon" CACHE PATH "Path to PyroCommon")
        if(NOT EXISTS "${PYRO_COMMON_DIR}/CMakeLists.txt")
            message(FATAL_ERROR "PyroCommon not found at ${PYRO_COMMON_DIR}")
        endif()
        set(PYRO_COMMON_BINARY_DIR "${CMAKE_BINARY_DIR}/PyroCommon")
        add_subdirectory(${PYRO_COMMON_DIR} ${PYRO_COMMON_BINARY_DIR})
    endif()
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
   
    set(D3D12MA_AGILITY_SDK_DIRECTORY ${DX12_AGILITY_SDK_PATH})
    # d3d12ma
    set(D3D12MA_BUILD_SAMPLE OFF)
    add_subdirectory(D3D12MemoryAllocator)
endif()
