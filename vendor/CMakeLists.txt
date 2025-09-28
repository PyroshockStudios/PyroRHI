# Common
add_subdirectory(${PYRO_COMMON_DIR} ${PYRO_COMMON_BINARY_DIR})

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