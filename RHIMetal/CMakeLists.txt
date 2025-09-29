set(SH_SRC ${CMAKE_SOURCE_DIR}/RHIMetal)
file(GLOB_RECURSE ENDF6_SRC
      "${SH_SRC}/*.hpp"
      "${SH_SRC}/*.cpp"
      "${SH_SRC}/*.mm")

add_library(RHIMetal SHARED ${ENDF6_SRC})

foreach(_source IN ITEMS ${ENDF6_SRC})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${SH_SRC}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()


target_compile_features(RHIMetal PRIVATE cxx_std_23)

target_link_libraries(RHIMetal PRIVATE PyroRHI::PyroRHI)
