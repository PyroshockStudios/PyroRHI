set(SH_SRC ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ENDF6_SRC
      "${SH_SRC}/*.hpp"
      "${SH_SRC}/*.cpp")

add_library(RHIDX12 SHARED ${ENDF6_SRC})

foreach(_source IN ITEMS ${ENDF6_SRC})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${SH_SRC}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

target_compile_features(RHIDX12 PRIVATE cxx_std_23)

target_link_libraries(RHIDX12 PRIVATE 
    PyroRHI::PyroRHI DirectX-Headers DirectX-Guids D3D12MemoryAllocator
    d3d12.lib d3dcompiler.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib
)
