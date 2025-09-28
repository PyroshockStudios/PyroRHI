set(SH_SRC ${CMAKE_SOURCE_DIR}/RHIVulkan)
file(GLOB_RECURSE ENDF6_SRC
      "${SH_SRC}/*.hpp"
      "${SH_SRC}/*.cpp")

add_library(RHIVulkan SHARED ${ENDF6_SRC})

foreach(_source IN ITEMS ${ENDF6_SRC})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${SH_SRC}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

target_compile_features(RHIVulkan PRIVATE cxx_std_23)

target_link_libraries(RHIVulkan PRIVATE PyroRHI::PyroRHI volk::volk VulkanMemoryAllocator)

if (UNIX AND NOT APPLE) 
# needed for surface creation
target_link_libraries(RHIVulkan PRIVATE 
wayland-client
xkbcommon
)
endif()
