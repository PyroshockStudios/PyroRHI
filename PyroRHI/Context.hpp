#pragma once
#include <PyroCommon/Core.hpp>
#include <PyroRHI/Core.hpp>
#include <PyroRHI/Api/Forward.hpp>
#include <PyroRHI/Shader/Forward.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        struct IShaderFeatureSet;
        enum struct RHIViewportConvention {
            None,
            LeftHanded_OriginTopLeft,
            LeftHanded_OriginBottomLeft,
            RightHanded_OriginTopLeft,
            RightHanded_OriginBottomLeft
        };
        struct RHIProperties {
            // this might be the only thing that is actually mandatory, unless we have some other path for the rendergraph... 
            bool bBufferDeviceAddress = false;
            bool bScalarLayout = false;
            bool bDrawIndirectCount = false;
            bool bUint8IndexBuffer = false;
            RHIViewportConvention viewportConvention = RHIViewportConvention::None;
            u32 bufferImageRowAlignment = 0;
        };

        class RHIContext : DeleteCopy, DeleteMove {
        public:
            RHIContext() {}
            virtual ~RHIContext() {}

            virtual IDevice* CreateDevice() = 0;

            virtual const RHIProperties& Properties() = 0;
            virtual IShaderFeatureSet* ShaderFeatureSet() = 0;
        };
    }
}