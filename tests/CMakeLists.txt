cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723 # release-1.17.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set_target_properties(gtest PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set_target_properties(gtest_main PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set_target_properties(gmock PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set_target_properties(gmock_main PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

if (PYRO_RHI_BUILD_VULKAN_VALIDATION_TESTS OR PYRO_RHI_BUILD_DX12_VALIDATION_TESTS)

function(addSlangDependencies targetName)

# get slang
file(GLOB SLANG_COPYLIBS
	"${CMAKE_BINARY_DIR}/_deps/slang/bin/*.dll"
	"${CMAKE_BINARY_DIR}/_deps/slang/bin/*.exe"
	"${CMAKE_BINARY_DIR}/_deps/slang/bin/*.slang"
)
message(STATUS "SLANG_COPYLIBS = ${SLANG_COPYLIBS}")
  # copy slang binaries
foreach(slang_lib ${SLANG_COPYLIBS})
	add_custom_command(TARGET ${targetName} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${slang_lib} $<TARGET_FILE_DIR:${targetName}>
)
endforeach()

endfunction()

add_subdirectory(RHIBaseTest)

include(FetchContent)
    set(SLANG_VERSION "2025.21") 
    set(SLANG_ROOT "${CMAKE_BINARY_DIR}/_deps/slang")
    set(SLANG_URL_BASE "https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}")

    if(WIN32)
        set(SLANG_ARCHIVE "slang-${SLANG_VERSION}-windows-x86_64.zip")
    elseif(UNIX AND NOT APPLE)
        set(SLANG_ARCHIVE "slang-${SLANG_VERSION}-linux-x86_64-glibc-2.27.zip") # use older glibc version
    elseif(APPLE)
        # Detect architecture: x86_64 or arm64
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE ARCHITECTURE
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(ARCHITECTURE STREQUAL "x86_64")
            set(SLANG_ARCHIVE "slang-${SLANG_VERSION}-macos-x86_64.zip")
        elseif(ARCHITECTURE STREQUAL "arm64")
            set(SLANG_ARCHIVE "slang-${SLANG_VERSION}-macos-aarch64.zip")
        else()
            message(FATAL_ERROR "Unsupported macOS architecture: ${ARCHITECTURE}")
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()

    set(SLANG_URL "${SLANG_URL_BASE}/${SLANG_ARCHIVE}")
    set(SLANG_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/_downloads")
    set(SLANG_EXTRACT_DIR "${SLANG_ROOT}")

    if(NOT EXISTS "${SLANG_EXTRACT_DIR}/include/slang.h")
        message(STATUS "Downloading Slang SDK from ${SLANG_URL}...")
        file(DOWNLOAD "${SLANG_URL}" "${SLANG_DOWNLOAD_DIR}/${SLANG_ARCHIVE}" SHOW_PROGRESS)
        message(STATUS "Extracting Slang SDK...")
        file(MAKE_DIRECTORY "${SLANG_EXTRACT_DIR}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${SLANG_DOWNLOAD_DIR}/${SLANG_ARCHIVE}"
            WORKING_DIRECTORY "${SLANG_EXTRACT_DIR}"
            RESULT_VARIABLE _extract_result
        )
        if(NOT _extract_result EQUAL 0)
            message(FATAL_ERROR "Failed to extract Slang SDK archive")
        endif()
    endif()

    set(SLANG_LIBS slang )
    if(UNIX AND NOT APPLE)
        list(APPEND SLANG_LIBS pthread dl)
    endif()

endif()

if (PYRO_RHI_BUILD_VULKAN_VALIDATION_TESTS OR PYRO_RHI_BUILD_DX12_VALIDATION_TESTS)
    add_subdirectory(RHITestChassis)
    target_include_directories(RHITestChassis PUBLIC "${SLANG_EXTRACT_DIR}/include")
    target_link_directories(RHITestChassis PUBLIC "${SLANG_EXTRACT_DIR}/lib")
    target_link_libraries(RHITestChassis PUBLIC ${SLANG_LIBS})
endif()

if (PYRO_RHI_BUILD_VULKAN_VALIDATION_TESTS)
    add_subdirectory(RHIValidationTestVulkan)
endif()

if (PYRO_RHI_BUILD_DX12_VALIDATION_TESTS)
    add_subdirectory(RHIValidationTestDX12)
endif()
