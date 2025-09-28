#pragma once
#include <EASTL/span.h>
#include <EASTL/string.h>

#include <PyroRHI/Shader/ShaderProgram.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        enum struct ShaderCompileTarget : i32 {
            Unknown,
            None,
            Glsl,
            Glsl_vulkan_deprecated,          //< deprecated and removed: just use `slang_glsl`.
            Glsl_vulkan_one_desc_deprecated, //< deprecated and removed.
            Hlsl,
            Spirv,
            SpirvAsm,
            Dxbc,
            DxbcAsm,
            Dxil,
            DxilAsm,
            C,                   ///< the c language
            Cpp,                 ///< c++ code for shader kernels.
            Host_executable,     ///< standalone binary executable (for hosting cpu/os)
            ShaderSharedLibrary, ///< a shared library/dll for shader kernels (for hosting
                                 ///< cpu/os)
            ShaderHostCallable,  ///< a cpu target that makes the compiled shader code available
                                 ///< to be run immediately
            CudaSource,          ///< cuda source
            Ptx,                 ///< ptx
            CudaObjectCode,      ///< object code that contains cuda functions.
            ObjectCode,          ///< object code that can be used for later linking
            HostCppSource,       ///< c++ code for host library or executable.
            HostHostCallable,    ///< host callable host code (ie non kernel/shader)
            CppPytorchBinding,   ///< c++ pytorch binding code.
            Metal,               ///< metal shading language
            MetalLib,            ///< metal library
            MetalLibAsm,         ///< metal library assembly
            HostSharedLibrary,   ///< a shared library/dll for host code (for hosting cpu/os)
            Wgsl,                ///< webgpu shading language
            WgslSpirvAsm,        ///< spir-v assembly via webgpu shading language
            WgslSpirv,           ///< spir-v via webgpu shading language

            HostVm, ///< bytecode that can be interpreted by the slang vm
        };
        struct ShaderFeatureInfo {
            bool bDescriptorIndexing = false;
            bool bBufferDeviceAddress = false;
            bool bScalarLayout = false;
            bool bDrawParameters = false;
            bool bGLSL = false;
        };
        struct IShaderFeatureSet {
            IShaderFeatureSet() = default;
            virtual ~IShaderFeatureSet() = default;

            PYRO_NODISCARD virtual ShaderCompileTarget GetTarget() const = 0;
            PYRO_NODISCARD virtual const char* GetProfileName(ShaderStage stage) const = 0;
            PYRO_NODISCARD virtual const char* GetFileExtension() const = 0;
            PYRO_NODISCARD virtual const ShaderFeatureInfo& Features() const = 0;

            PYRO_NODISCARD virtual const eastl::span<eastl::pair<const char*, const char*>>& GlobalPreprocessorDefines() const = 0;
        };

    }
}