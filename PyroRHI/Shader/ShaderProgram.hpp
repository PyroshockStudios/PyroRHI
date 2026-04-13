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
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Core.hpp>
#include <PyroRHI/Shader/Forward.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        struct ShaderReflection;
        enum struct ShaderStage : i32 {
            None,
            Vertex,   // vsh
            Hull,     // hsh AKA vulkan tesselation control
            Domain,   // dsh AKA vulkan tesselation evaluation
            Geometry, // gsh
            Fragment, // fsh/psh
            Compute,  // csh

            RayGeneration,   // rgen
            RayIntersection, // rint
            RayAnyHit,       // rany
            RayClosestHit,   // rhit
            RayMiss,         // rmis
            Callable,        // call
            Mesh,            // mesh
            Amplification,   // ampl
            Dispatch,        // disp

            // aliases
            TesselationControl = Hull,
            TesselationEvaluation = Domain,
            Pixel = Fragment
        };
        PYRO_NODISCARD static ShaderStage ShaderStageFromShorthand(const eastl::string_view& str) {
            if (str == "vsh")
                return ShaderStage::Vertex;
            if (str == "hsh")
                return ShaderStage::Hull;
            if (str == "dsh")
                return ShaderStage::Domain;
            if (str == "gsh")
                return ShaderStage::Geometry;
            if (str == "fsh" || str == "psh")
                return ShaderStage::Fragment;
            if (str == "csh")
                return ShaderStage::Compute;

            if (str == "rgen")
                return ShaderStage::RayGeneration;
            if (str == "rint")
                return ShaderStage::RayIntersection;
            if (str == "rany")
                return ShaderStage::RayAnyHit;
            if (str == "rhit")
                return ShaderStage::RayClosestHit;
            if (str == "rmis")
                return ShaderStage::RayMiss;
            if (str == "call")
                return ShaderStage::Callable;
            if (str == "mesh")
                return ShaderStage::Mesh;
            if (str == "ampl")
                return ShaderStage::Amplification;
            if (str == "disp")
                return ShaderStage::Dispatch;

            return ShaderStage::None;
        }

        struct ShaderProgram {
            eastl::vector<u8> bytecode = {};

            PYRO_NODISCARD bool operator==(const ShaderProgram&) const = default;
            PYRO_NODISCARD bool operator!=(const ShaderProgram&) const = default;
            PYRO_FORCEINLINE operator bool() const {
                return !bytecode.empty();
            }
        };
    } // namespace RHI
} // namespace PyroshockStudios