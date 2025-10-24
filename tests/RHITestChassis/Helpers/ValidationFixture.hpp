#pragma once

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN 1
#endif

#ifndef RHI_TEST_CHASSIS_API_LIB
#error Missing RHI_TEST_CHASSIS_API_LIB
#endif
#ifndef RHI_CONTEXT_FIXTURE_NAME
#error Missing RHI_CONTEXT_FIXTURE_NAME
#endif
#ifndef RHI_TEST_CHASSIS_API_LOG_NAME
#error Missing RHI_TEST_CHASSIS_API_LOG_NAME
#endif
#ifndef RHI_TEST_CHASSIS_API_VALIDATOR_NAME
#error Missing RHI_TEST_CHASSIS_API_VALIDATOR_NAME
#endif
#ifndef RHI_TEST_CHASSIS_RHI_OPTIONS
#error Missing RHI_TEST_CHASSIS_RHI_OPTIONS
#endif

#include <PyroCommon/Logger.hpp>
#include <PyroRHI/Api/IDevice.hpp>
#include <PyroRHI/Api/ToString.hpp>
#include <PyroRHI/Context.hpp>
#include <PyroRHI/Exports.hpp>
#include <filesystem>
#include <gtest/gtest.h>

#if defined(_WIN32)
#include <Windows.h>
#define LOAD_LIB(path) LoadLibraryA(path)
#define GET_SYM(lib, name) GetProcAddress((HMODULE)(lib), name)
#define CLOSE_LIB(lib) FreeLibrary((HMODULE)(lib))
using LibraryHandle = HMODULE;
#else
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#define LOAD_LIB(path) dlopen(path, RTLD_NOW | RTLD_LOCAL)
#define GET_SYM(lib, name) dlsym(lib, name)
#define CLOSE_LIB(lib) dlclose(lib)
using LibraryHandle = void*;
#endif
#ifdef PYRO_PLATFORM_FAMILY_APPLE
#include <mach-o/dyld.h>
#endif

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

struct UsedMemberInfo {
    const char* dataName;
    eastl::string stringified;
};

#define TRACK_RHI_PARAMETER(infoStruct) mUsedData.emplace_back(#infoStruct, infoStruct.ToString())
#define TRACK_RHI_HANDLE(handle) mUsedData.emplace_back(#handle, eastl::string().sprintf("0x%016llX", eastl::bit_cast<u64>(handle)))

class RHI_CONTEXT_FIXTURE_NAME : public ::testing::Test, public ILogStream {
    struct SecondaryLogStream : public ILogStream {
        void Log(LogSeverity severity, const char* message) override {
            if (severity == LogSeverity::Error) {
                if (pFailed)
                    *pFailed = true;
                ADD_FAILURE() << "[" RHI_TEST_CHASSIS_API_LOG_NAME "] "
                                 "Implementation Error : "
                              << message;
            } else if (severity == LogSeverity::Fatal) {
                if (pFailed)
                    *pFailed = true;
                GTEST_FAIL() << "[" RHI_TEST_CHASSIS_API_LOG_NAME "] "
                                "FATAL IMPLEMENTATION ERROR : "
                             << message;
            } else {
                std::cout << "[" RHI_TEST_CHASSIS_API_LOG_NAME "] " << message << "\n";
            }
        }
        LogSeverity MinSeverity() const override {
            // only care about catching errors
            return LogSeverity::Debug;
        }
        const char* Name() const override {
            return RHI_TEST_CHASSIS_API_LOG_NAME;
        }
        SecondaryLogStream(bool* p) : pFailed(p) {}
        bool* pFailed = nullptr;
    };
    void Log(LogSeverity severity, const char* message) override {
        if (severity >= LogSeverity::Error) {
            bFailed = true;
            ADD_FAILURE() << "[" RHI_TEST_CHASSIS_API_VALIDATOR_NAME "] Validation Error: " << message;
        } else {
            std::cout << "[" RHI_TEST_CHASSIS_API_VALIDATOR_NAME "] " << message << "\n";
        }
    }
    LogSeverity MinSeverity() const override {
        // only care about catching validation errors
        return LogSeverity::Info;
    }
    const char* Name() const override {
        return "VVL";
    }

    bool bFailed = false;

protected:
    LibraryHandle mLibrary = nullptr;
    RHIInfo mInfo = {};
    RHIContextApiInfo mApi = {};
    RHICreateInfo mCreateInfo = {};
    ILogStream* mLogger = {};
    SecondaryLogStream mLogStream2 = { &bFailed };
    IDevice* mDevice = {};

    // Function pointers from the dynamically loaded RHI
    PFN_GetCustomRHIInfo fpGetInfo = nullptr;
    PFN_CreateRHIContext fpCreateContext = nullptr;
    PFN_DestroyRHIContext fpDestroyContext = nullptr;

    eastl::vector<UsedMemberInfo> mUsedData;

    void SetUp() override {
#ifdef PYRO_PLATFORM_WINDOWS
        const char* libName = "RHI/" RHI_TEST_CHASSIS_API_LIB ".dll";
#elif defined(PYRO_PLATFORM_FAMILY_APPLE)
        const char* libName = "RHI/lib" RHI_TEST_CHASSIS_API_LIB ".dylib";
#elif defined(PYRO_PLATFORM_FAMILY_UNIX)
        const char* libName = "RHI/lib" RHI_TEST_CHASSIS_API_LIB ".so";
#endif
        eastl::string exeDir = {};

#ifdef PYRO_PLATFORM_WINDOWS
        char path[256];
        DWORD size = GetModuleFileNameA(nullptr, path, sizeof(path));
        if (size == 0 || size == sizeof(path))
            GTEST_FAIL() << "Failed to get executable path";
        std::filesystem::path exePath(path);
        exeDir = eastl::string(exePath.parent_path().string().c_str());
#elif defined(PYRO_PLATFORM_MACOS)
        char path[1024];
        u32 size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) != 0)
            GTEST_FAIL() << "Failed to get executable path";
        std::filesystem::path exePath(path);
        exeDir = eastl::string(exePath.parent_path().string().c_str());
#elif defined(PYRO_PLATFORM_LINUX)
        char path[1024];
        isize size = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (size <= 0)
            GTEST_FAIL() << "Failed to get executable path";
        path[size] = '\0';
        std::filesystem::path exePath = std::filesystem::path(std::string(path));
        exeDir = eastl::string(exePath.parent_path().string().c_str());
#endif
        eastl::string fullPath = exeDir + "/" + libName;

        mLibrary = LOAD_LIB(fullPath.c_str());
        ASSERT_NE(mLibrary, nullptr) << "Failed to load RHI backend: " << libName;

        fpGetInfo = reinterpret_cast<decltype(fpGetInfo)>(GET_SYM(mLibrary, "GetCustomRHIInfo"));
        fpCreateContext = reinterpret_cast<decltype(fpCreateContext)>(GET_SYM(mLibrary, "CreateRHIContext"));
        fpDestroyContext = reinterpret_cast<decltype(fpDestroyContext)>(GET_SYM(mLibrary, "DestroyRHIContext"));

        ASSERT_NE(fpGetInfo, nullptr);
        ASSERT_NE(fpCreateContext, nullptr);
        ASSERT_NE(fpDestroyContext, nullptr);

        fpGetInfo(&mInfo);
        ASSERT_TRUE(mInfo.guid.Valid()) << "Invalid RHI GUID returned";
        ASSERT_GT(strlen(mInfo.name), 0) << "RHI name should not be empty";

        memset(&mCreateInfo, 0, sizeof(RHICreateInfo));
        mCreateInfo.appName = "ValidationTest " RHI_TEST_CHASSIS_API_LOG_NAME;
        mCreateInfo.engineName = "NO ENGINE";
        mCreateInfo.appVersion = 100000000U;
        mCreateInfo.engineVersion = 100000000U;
        mCreateInfo.pDebugSink = this;
        mCreateInfo.pLoggerSink = &mLogStream2;

        // Enable validation + headless mode if supported
        RHI_TEST_CHASSIS_RHI_OPTIONS

        fpCreateContext(&mCreateInfo, &mApi);
        ASSERT_NE(mApi.loadedContext, nullptr) << "Failed to create " RHI_TEST_CHASSIS_API_LOG_NAME " Context";

        mDevice = mApi.loadedContext->CreateDevice();
        ASSERT_NE(mDevice, nullptr) << "Failed to create " RHI_TEST_CHASSIS_API_LOG_NAME " Device";
    }

    void TearDown() override {
        if (bFailed) {
            GTEST_LOG_(INFO) << "[" RHI_TEST_CHASSIS_API_LOG_NAME "] HAS FAILED! Used parameters:\n";
            for (auto& d : mUsedData) {
                GTEST_LOG_(INFO) << "  - " << d.dataName << ":\n"
                          << d.stringified.c_str();
            }
            GTEST_LOG_(INFO) << "\n";
        }
        // Do not wait idle, some tests also have an extra check where device idle shouldn't be necessary!
        // mDevice->WaitIdle();
        mDevice = nullptr;
        if (fpDestroyContext && mApi.loadedContext)
            fpDestroyContext(&mApi);

        if (mLibrary)
            CLOSE_LIB(mLibrary);
    }
};
