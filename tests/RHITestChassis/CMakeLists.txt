set(SH_SRC ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ENDF6_SRC
      "${SH_SRC}/*.hpp"
      "${SH_SRC}/*.cpp")

add_library(RHITestChassis STATIC ${ENDF6_SRC})

foreach(_source IN ITEMS ${ENDF6_SRC})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${SH_SRC}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

target_include_directories(RHITestChassis
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DESTINATION include/RHITestChassis
    FILES_MATCHING PATTERN "*.hpp"
)

target_compile_features(RHITestChassis PUBLIC cxx_std_23)

target_link_libraries(RHITestChassis PUBLIC PyroRHI::PyroRHI)
