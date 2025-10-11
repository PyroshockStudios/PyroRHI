option(PYRO_RHI_ENABLE_CODE_COVERAGE "Enable code coverage reporting" OFF)

# Coverage flags for platforms:
set(_COV_COMPILE_FLAGS "")
set(_COV_LINK_FLAGS "")

if(PYRO_RHI_ENABLE_CODE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32)
        message(STATUS "Building with code coverage enabled (Linux / GCC/Clang)")
        # GCC/Clang on Linux -> gcov/lcov style
        set(_COV_COMPILE_FLAGS -O0 -g -fprofile-arcs -ftest-coverage)
        set(_COV_LINK_FLAGS --coverage)

    elseif(WIN32)
        message(STATUS "Building with code coverage enabled (Windows clang-cl / LLVM coverage)")
        # clang-cl style flags (use MSVC-style slash for clang-cl compatibility)
        # Use /Zi and /Od for debug/no-optim, and /fprofile-instr-generate and /fcoverage-mapping for LLVM
        set(_COV_COMPILE_FLAGS /Zi /Od /fprofile-instr-generate /fcoverage-mapping)
        # link flags: ensure we keep debug info; use clang flags too
        set(_COV_LINK_FLAGS /DEBUG /fprofile-instr-generate)
    else()
        message(FATAL_ERROR "Code coverage requested, but compiler/platform is not supported: ${CMAKE_CXX_COMPILER_ID} / ${CMAKE_SYSTEM_NAME}")
    endif()
endif()

# Helper function to instrument ONLY the specified target(s)
function(add_coverage_instrumentation target)
    if(NOT PYRO_RHI_ENABLE_CODE_COVERAGE)
        message(STATUS "Coverage not enabled; skipping instrumentation for target ${target}")
        return()
    endif()

    if(NOT TARGET ${target})
        message(FATAL_ERROR "add_coverage_instrumentation: target '${target}' does not exist")
    endif()

    if(_COV_COMPILE_FLAGS)
        foreach(flag IN LISTS _COV_COMPILE_FLAGS)
            target_compile_options(${target} PRIVATE ${flag})
        endforeach()
    endif()

    if(_COV_LINK_FLAGS)
        foreach(flag IN LISTS _COV_LINK_FLAGS)
            # target_link_options requires CMake >= 3.13; fallback to PROPERTY if needed
            if(COMMAND target_link_options)
                target_link_options(${target} PRIVATE ${flag})
            else()
                get_target_property(_old ${target} LINK_FLAGS)
                if(NOT _old) 
                    set(_old "")
                endif()
                set_target_properties(${target} PROPERTIES LINK_FLAGS "${_old} ${flag}")
            endif()
        endforeach()
    endif()
endfunction()