# Common

# Default path for PyroCommon if user does not provide one
if(NOT TARGET PyroCommon)
    include(FetchContent)
    # find the package locally first
    find_package(PyroCommon CONFIG QUIET)
    # If not found, fetch it directly from repository
    if(NOT PyroCommon_FOUND)
        message(STATUS "PyroCommon not found locally. Fetching from GitHub...")
        FetchContent_Declare(
            PyroCommon
            GIT_REPOSITORY https://github.com/PyroshockStudios/PyroCommon.git
            GIT_TAG        master
        )
        FetchContent_MakeAvailable(PyroCommon)
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
