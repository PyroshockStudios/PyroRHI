#pragma once

#include <PyroCommon/Logger.hpp>
#include <PyroRHI/Context.hpp>
#include <PyroRHI/Exports.hpp>
#include <PyroRHI/Api/IDevice.hpp>
#include <gtest/gtest.h>

#if defined(_WIN32)
#include <windows.h>
#define LOAD_LIB(path) LoadLibraryA(path)
#define GET_SYM(lib, name) GetProcAddress((HMODULE)(lib), name)
#define CLOSE_LIB(lib) FreeLibrary((HMODULE)(lib))
using LibraryHandle = HMODULE;
#else
#include <dlfcn.h>
#define LOAD_LIB(path) dlopen(path, RTLD_NOW)
#define GET_SYM(lib, name) dlsym(lib, name)
#define CLOSE_LIB(lib) dlclose(lib)
using LibraryHandle = void*;
#endif

namespace PyroshockStudios::RHI::Tests {
    class VulkanContextFixture : public ::testing::Test, public ILogStream {
        struct SecondaryLogStream : public ILogStream {
            void Log(LogSeverity severity, const char* message) override {
                if (severity == LogSeverity::Error) {
                    ADD_FAILURE() << "[VULKAN] Implementation Error: " << message;
                } else if (severity == LogSeverity::Fatal) {
                    GTEST_FAIL() << "[VULKAN] FATAL IMPLEMENTATION ERROR: " << message;
                }
            }
            LogSeverity MinSeverity() const override {
                // only care about catching errors
                return LogSeverity::Error;
            }
            const char* Name() const override {
                return "Vulkan";
            }
        };
        void Log(LogSeverity severity, const char* message) override {
            if (severity >= LogSeverity::Error) {
                ADD_FAILURE() << "[VVL] Validation Error: " << message;
            }
        }
        LogSeverity MinSeverity() const override {
            // only care about catching validation errors
            return LogSeverity::Error;
        }
        const char* Name() const override {
            return "VVL";
        }

    protected:
        LibraryHandle mLibrary = nullptr;
        RHIInfo mInfo = {};
        RHIContextApiInfo mApi = {};
        RHICreateInfo mCreateInfo = {};
        ILogStream* mLogger = {};
        SecondaryLogStream mLogStream2 = {};
        IDevice* mDevice = {};

        // Function pointers from the dynamically loaded RHI
        PFN_GetCustomRHIInfo fpGetInfo = nullptr;
        PFN_CreateRHIContext fpCreateContext = nullptr;
        PFN_DestroyRHIContext fpDestroyContext = nullptr;

        void SetUp() override {
#ifdef PYRO_PLATFORM_WINDOWS
            const char* libName = "RHI/RHIVulkan.dll";
#elif defined(PYRO_PLATFORM_FAMILY_APPLE)
            const char* libName = "RHI/RHIVulkan.dylib";
#elif defined(PYRO_PLATFORM_FAMILY_UNIX)
            const char* libName = "RHI/RHIVulkan.so";
#endif
            mLibrary = LOAD_LIB(libName);
            ASSERT_NE(mLibrary, nullptr) << "Failed to load RHI backend: " << libName;

            fpGetInfo = reinterpret_cast<decltype(fpGetInfo)>(GET_SYM(mLibrary, "GetCustomRHIInfo"));
            fpCreateContext = reinterpret_cast<decltype(fpCreateContext)>(GET_SYM(mLibrary, "CreateRHIContext"));
            fpDestroyContext = reinterpret_cast<decltype(fpDestroyContext)>(GET_SYM(mLibrary, "DestroyRHIContext"));

            ASSERT_NE(fpGetInfo, nullptr);
            ASSERT_NE(fpCreateContext, nullptr);
            ASSERT_NE(fpDestroyContext, nullptr);

            fpGetInfo(&mInfo);
            ASSERT_NE(mInfo.guid, GUID{}) << "Invalid RHI GUID returned";
            ASSERT_GT(strlen(mInfo.name), 0) << "RHI name should not be empty";


            memset(&mCreateInfo, 0, sizeof(RHICreateInfo));
            mCreateInfo.appName = "VulkanValidationTest";
            mCreateInfo.engineName = "NO ENGINE";
            mCreateInfo.appVersion = 100000000U;
            mCreateInfo.engineVersion = 100000000U;
            mCreateInfo.pDebugSink = this;
            mCreateInfo.pLoggerSink = &mLogStream2;

            // Enable validation + headless mode if supported
            mCreateInfo.options[0] = { .optionIndex = 0 }; // "debug"
            mCreateInfo.options[1] = { .optionIndex = 1 }; // "headless"

            fpCreateContext(&mCreateInfo, &mApi);
            ASSERT_NE(mApi.loadedContext, nullptr) << "Failed to create Vulkan Context";

            mDevice = mApi.loadedContext->CreateDevice();
            ASSERT_NE(mDevice, nullptr) << "Failed to create Vulkan Device";
        }

        void TearDown() override {
            mDevice->WaitIdle();
            mDevice = nullptr;
            if (fpDestroyContext && mApi.loadedContext)
                fpDestroyContext(&mApi);

            if (mLibrary)
                CLOSE_LIB(mLibrary);
        }
    };

} // namespace PyroshockStudios::RHI::Tests
