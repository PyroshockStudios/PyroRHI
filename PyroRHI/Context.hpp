// MIT License
//
// Copyright (c) 2025 Pyroshock Studios
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include <PyroCommon/Core.hpp>
#include <PyroCommon/LoggerInterface.hpp>
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

        class RHIContext : ILoggerAware, DeleteCopy, DeleteMove {
        public:
            RHIContext() {}
            virtual ~RHIContext() {}

            virtual IDevice* CreateDevice() = 0;

            virtual const RHIProperties& Properties() = 0;
            virtual IShaderFeatureSet* ShaderFeatureSet() = 0;
        };
    }
}