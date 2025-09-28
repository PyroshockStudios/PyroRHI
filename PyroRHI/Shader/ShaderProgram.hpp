#pragma once
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Core.hpp>
#include <PyroRHI/Shader/Forward.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        class ShaderReflection;
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