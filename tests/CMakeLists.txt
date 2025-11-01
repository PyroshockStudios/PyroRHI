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
FetchContent_MakeAvailable(googletest)

set_target_properties(gtest PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set_target_properties(gtest_main PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set_target_properties(gmock PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set_target_properties(gmock_main PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_subdirectory(RHITestChassis)
add_subdirectory(RHIBaseTest)

if (PYRO_RHI_BUILD_VULKAN_VALIDATION_TESTS)
    add_subdirectory(RHIValidationTestVulkan)
endif()

if (PYRO_RHI_BUILD_DX12_VALIDATION_TESTS)
    add_subdirectory(RHIValidationTestDX12)
endif()