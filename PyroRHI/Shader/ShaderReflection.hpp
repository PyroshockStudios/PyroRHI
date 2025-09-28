#pragma once
#include <EASTL/optional.h>
#include <EASTL/span.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <PyroCommon/Core.hpp>
#include <PyroCommon/Serialization/BinarySerializer.hpp>
#include <PyroCommon/Serialization/ISerializable.hpp>
#include <PyroRHI/Core.hpp>


namespace PyroshockStudios {
    inline namespace RHI {
        enum struct ShaderInputType : i32 {
            Unknown,

            UniformBuffer,

            ByteAddressBuffer,
            RWByteAddressBuffer,

            StructuredBuffer,
            RWStructuredBuffer,

            Texture1D,
            Texture2D,
            TextureCube,
            Texture3D,
            Texture1DArray,
            Texture2DArray,
            TextureCubeArray,
            Texture2DMS,
            Texture2DMSArray,

            RWTexture1D,
            RWTexture2D,
            RWTexture3D,
            RWTexture1DArray,
            RWTexture2DArray,

            Sampler,
            // SamplerComparison,

            PushConstant,

            AccelerationStructure
        };
        enum struct ShaderScalarType : i32 {
            None,
            Void,
            Bool,
            Int32,
            UInt32,
            Int64,
            UInt64,
            Float16,
            Float32,
            Float64,
            Int8,
            UInt8,
            Int16,
            UInt16,
            Array,
            Struct
        };
        struct ShaderStructureInfo : ISerializable {
            eastl::string name = {};
            ShaderScalarType type = ShaderScalarType::None;
            u32 offset = 0;       // Byte offset from base
            u32 size = 0;         // Total size in bytes
            u32 stride = 0;       // If an array/matrix
            u32 rowCount = 0;     // If a matrix
            u32 columnCount = 0;  // If a matrix
            u32 elementCount = 0; // if vector, returns the dimensions. If an array, it returns the number of elements
            bool bRowMajor = false;
            eastl::optional<eastl::vector<ShaderStructureInfo>> members = {}; // for structs

            PYRO_NODISCARD bool IsScalar() const {
                return !IsArray() && !IsVector() && !IsStruct() && !IsMatrix();
            }
            PYRO_NODISCARD bool IsArray() const {
                return type == ShaderScalarType::Array;
            }
            PYRO_NODISCARD eastl::optional<ShaderStructureInfo> ArrayUndelyingType() const {
                return members.has_value() ? (*members)[0] : eastl::optional<ShaderStructureInfo>{};
            }
            PYRO_NODISCARD bool IsVector() const {
                return !IsArray() && elementCount > 1;
            }
            PYRO_NODISCARD bool IsMatrix() const {
                return rowCount > 1 && columnCount > 1;
            }
            PYRO_NODISCARD bool IsStruct() const {
                return type == ShaderScalarType::Struct;
            }

            void Serialize(BinarySerializer& serializer) const override {
                serializer << type << name << offset << size << stride << rowCount << columnCount << elementCount << bRowMajor << members;
            }
            void Deserialize(BinarySerializer& serializer) override {
                serializer >> type >> name >> offset >> size >> stride >> rowCount >> columnCount >> elementCount >> bRowMajor >> members;
            }

            PYRO_NODISCARD bool operator==(const ShaderStructureInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const ShaderStructureInfo&) const = default;
        };

        struct ShaderInputInfo : ISerializable {
            eastl::string name = {};
            ShaderInputType type = {};
            u32 space = 0;
            u32 binding = 0;
            // in bytes, if applicable
            u32 size = 0;
            // for structs and buffers
            eastl::optional<eastl::vector<ShaderStructureInfo>> members = {};

            void Serialize(BinarySerializer& serializer) const override {
                serializer << name << type << space << binding << size << members;
            }
            void Deserialize(BinarySerializer& serializer) override {
                serializer >> name >> type >> space >> binding >> size >> members;
            }

            PYRO_NODISCARD bool operator==(const ShaderInputInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const ShaderInputInfo&) const = default;
        };
        struct ShaderReflection : ISerializable {
        private:
            eastl::string HelperToString(ShaderScalarType type) {
                switch (type) {
                case ShaderScalarType::None:
                    return "None";
                case ShaderScalarType::Void:
                    return "Void";
                case ShaderScalarType::Bool:
                    return "Bool";
                case ShaderScalarType::Int32:
                    return "Int32";
                case ShaderScalarType::UInt32:
                    return "UInt32";
                case ShaderScalarType::Int64:
                    return "Int64";
                case ShaderScalarType::UInt64:
                    return "UInt64";
                case ShaderScalarType::Float16:
                    return "Float16";
                case ShaderScalarType::Float32:
                    return "Float32";
                case ShaderScalarType::Float64:
                    return "Float64";
                case ShaderScalarType::Int8:
                    return "Int8";
                case ShaderScalarType::UInt8:
                    return "UInt8";
                case ShaderScalarType::Int16:
                    return "Int16";
                case ShaderScalarType::UInt16:
                    return "UInt16";
                case ShaderScalarType::Array:
                    return "Array";
                case ShaderScalarType::Struct:
                    return "Struct";
                default:
                    return "Unknown";
                }
            }

            eastl::string HelperToString(ShaderInputType type) {
                switch (type) {
                case ShaderInputType::Unknown:
                    return "Unknown";

                case ShaderInputType::UniformBuffer:
                    return "UniformBuffer";

                case ShaderInputType::ByteAddressBuffer:
                    return "ByteAddressBuffer";
                case ShaderInputType::RWByteAddressBuffer:
                    return "RWByteAddressBuffer";

                case ShaderInputType::StructuredBuffer:
                    return "StructuredBuffer";
                case ShaderInputType::RWStructuredBuffer:
                    return "RWStructuredBuffer";

                case ShaderInputType::Texture1D:
                    return "Texture1D";
                case ShaderInputType::Texture2D:
                    return "Texture2D";
                case ShaderInputType::TextureCube:
                    return "TextureCube";
                case ShaderInputType::Texture3D:
                    return "Texture3D";
                case ShaderInputType::Texture1DArray:
                    return "Texture1DArray";
                case ShaderInputType::Texture2DArray:
                    return "Texture2DArray";
                case ShaderInputType::TextureCubeArray:
                    return "TextureCubeArray";
                case ShaderInputType::Texture2DMS:
                    return "Texture2DMS";
                case ShaderInputType::Texture2DMSArray:
                    return "Texture2DMSArray";

                case ShaderInputType::RWTexture1D:
                    return "RWTexture1D";
                case ShaderInputType::RWTexture2D:
                    return "RWTexture2D";
                case ShaderInputType::RWTexture3D:
                    return "RWTexture3D";
                case ShaderInputType::RWTexture1DArray:
                    return "RWTexture1DArray";
                case ShaderInputType::RWTexture2DArray:
                    return "RWTexture2DArray";

                case ShaderInputType::Sampler:
                    return "Sampler";
                    // case ShaderInputType::SamplerComparison:
                    //     return "SamplerComparison";

                case ShaderInputType::PushConstant:
                    return "PushConstant";
                case ShaderInputType::AccelerationStructure:
                    return "AccelerationStructure";

                default:
                    return "Invalid";
                }
            }

            static eastl::string Indent(u32 level) {
                return eastl::string(level * 4, ' '); // 4 spaces per indent level
            }

            eastl::string HelperToString(const ShaderStructureInfo& info, u32 indentLevel = 0);

            eastl::string HelperToString(const eastl::optional<eastl::vector<ShaderStructureInfo>>& members, u32 indentLevel) {
                if (!members.has_value() || members->empty()) {
                    return "";
                }
                eastl::string result;
                for (const auto& member : *members) {
                    result += HelperToString(member, indentLevel) + "\n";
                }
                return result;
            }

            eastl::string HelperToString(const ShaderStructureInfo& info, u32 indentLevel) {
                eastl::string indent = Indent(indentLevel);
                eastl::string result;

                // Name and type
                result += indent + "Name: " + info.name + "\n";
                result += indent + "Type: " + HelperToString(info.type) + "\n";
                result += indent + "Offset: " + to_string(info.offset) + " bytes\n";
                result += indent + "Size: " + to_string(info.size) + " bytes\n";

                // Layout details
                if (info.IsArray()) {
                    result += indent + "Array: " + to_string(info.elementCount) + " elements\n";
                    result += indent + "Array Stride: " + to_string(info.stride) + " bytes\n";
                    result += indent + "Array Type:\n";
                    result += HelperToString(*info.ArrayUndelyingType(), indentLevel + 1);
                }
                if (info.IsVector()) {
                    result += indent + "Vector Size: " + to_string(info.elementCount) + "\n";
                }
                if (info.IsMatrix()) {
                    result += indent + "Matrix: " + to_string(info.rowCount) + " rows x " + to_string(info.columnCount) + " columns\n";
                    result += indent + "Matrix Stride: " + to_string(info.stride) + " bytes\n";
                    result += indent + "Row Major: " + (info.bRowMajor ? "Yes" : "No") + "\n";
                }
                if (info.IsScalar()) {
                    result += indent + "Scalar\n";
                }
                if (info.IsStruct()) {
                    result += indent + "Struct Members:\n";
                    result += HelperToString(info.members, indentLevel + 1);
                }
                return result;
            }

            eastl::string HelperToString(const ShaderInputInfo& input, u32 indentLevel = 0) {
                eastl::string indent = Indent(indentLevel);
                eastl::string result;

                result += indent + "Name: " + input.name + "\n";
                result += indent + "Type: " + HelperToString(input.type) + "\n";
                result += indent + "Space: " + to_string(input.space) + "\n";
                result += indent + "Binding: " + to_string(input.binding) + "\n";
                if (input.size > 0) {
                    result += indent + "Size: " + to_string(input.size) + " bytes\n";
                }
                if (input.members.has_value() && !input.members->empty()) {
                    result += indent + "Members:\n";
                    result += HelperToString(input.members, indentLevel + 1);
                }
                return result;
            }

            eastl::string HelperToString(const eastl::vector<ShaderInputInfo>& inputs) {
                eastl::string result;
                for (const auto& input : inputs) {
                    result += HelperToString(input) + "\n";
                }
                return result;
            }

            eastl::string HelperToString(const eastl::optional<ShaderInputInfo>& pushConstant) {
                if (!pushConstant.has_value()) {
                    return "No Push Constant";
                }
                return HelperToString(*pushConstant, 1);
            }

        public:
            eastl::vector<ShaderInputInfo> inputs = {};
            eastl::optional<ShaderInputInfo> push = {};

            // Debug helper
            PYRO_NODISCARD  eastl::string ToString() const {
                eastl::string str =
                    "Shader Inputs:\n" + HelperToString(inputs) + "\n" +
                    "Push Constant:\n" + HelperToString(push);
                return str;
            }

            void Serialize(BinarySerializer& serializer) const override {
                serializer << inputs << push;
            }
            void Deserialize(BinarySerializer& serializer) override {
                serializer >> inputs >> push;
            }

            PYRO_NODISCARD bool operator==(const ShaderReflection&) const = default;
            PYRO_NODISCARD bool operator!=(const ShaderReflection&) const = default;
        };
    } // namespace RHI
} // namespace PyroshockStudios