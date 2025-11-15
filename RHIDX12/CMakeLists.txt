set(SH_SRC ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ENDF6_SRC
      "${SH_SRC}/*.hpp"
      "${SH_SRC}/*.cpp")

add_library(RHIDX12 SHARED ${ENDF6_SRC})

set_target_properties(RHIDX12 PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/RHI"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug/RHI"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release/RHI"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/RHI"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin/MinSizeRel/RHI"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/RHI"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug/RHI"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release/RHI"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/RHI"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin/MinSizeRel/RHI"
)

foreach(_source IN ITEMS ${ENDF6_SRC})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${SH_SRC}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

target_compile_features(RHIDX12 PRIVATE cxx_std_23)

target_link_libraries(RHIDX12 PRIVATE 
    PyroRHI::PyroRHI DirectX-Headers DirectX-Guids D3D12MemoryAllocator
    d3d12.lib d3dcompiler.lib dxcompiler.lib dxgi.lib dxguid.lib dcomp.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib
)

add_custom_target(BuildPyroImplRHIDX12 ALL DEPENDS RHIDX12)
set_target_properties(BuildPyroImplRHIDX12 PROPERTIES FOLDER "PyroBuildRHI")

# --- Agility SDK Configuration ---

target_include_directories(RHIDX12 PRIVATE "${DX12_AGILITY_SDK_INCLUDE_PATH}")
set(OUTPUT_SDK_DIR "$<TARGET_FILE_DIR:RHIDX12>/D3D12")

add_custom_command(
    TARGET RHIDX12 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_SDK_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DX12_AGILITY_SDK_BIN_PATH}/D3D12Core.dll"
        "${OUTPUT_SDK_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DX12_AGILITY_SDK_BIN_PATH}/d3d12SDKLayers.dll"
        "${OUTPUT_SDK_DIR}/"
    COMMENT "Copying DX12 Agility SDK DLLs"
)