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

#include <EASTL/span.h>
#include <EASTL/string.h>

#include <PyroCommon/Core.hpp>
#include <PyroRHI/Core.hpp>
#include <compare>

namespace PyroshockStudios {
    inline namespace RHI {
        struct IDevice;

        struct StatisticQueryPoolFlagsProperties {
            using Data = u32;
        };
        using StatisticQueryPoolFlags = Flags<StatisticQueryPoolFlagsProperties>;
        struct StatisticQueryPoolBits {
            static constexpr inline StatisticQueryPoolFlags NONE = { 0x00000000 };
            static constexpr inline StatisticQueryPoolFlags INPUT_ASSEMBLY_VERTICES = { 0x00000001 };
            static constexpr inline StatisticQueryPoolFlags INPUT_ASSEMBLY_PRIMITIVES = { 0x00000002 };
            static constexpr inline StatisticQueryPoolFlags VERTEX_SHADER_INVOCATIONS = { 0x00000004 };
            static constexpr inline StatisticQueryPoolFlags GEOMETRY_SHADER_INVOCATIONS = { 0x00000008 };
            static constexpr inline StatisticQueryPoolFlags GEOMETRY_SHADER_PRIMITIVES = { 0x00000010 };
            static constexpr inline StatisticQueryPoolFlags CLIPPING_INVOCATIONS = { 0x00000020 };
            static constexpr inline StatisticQueryPoolFlags CLIPPING_PRIMITIVES = { 0x00000040 };
            static constexpr inline StatisticQueryPoolFlags FRAGMENT_SHADER_INVOCATIONS = { 0x00000080 };
            static constexpr inline StatisticQueryPoolFlags TESSELLATION_CONTROL_SHADER_PATCHES = { 0x00000100 };
            static constexpr inline StatisticQueryPoolFlags TESSELLATION_EVALUATION_SHADER_INVOCATIONS = { 0x00000200 };
            static constexpr inline StatisticQueryPoolFlags COMPUTE_SHADER_INVOCATIONS = { 0x00000400 };
        };

        /**
        * @brief Info to create a query pool for timestamps
        */
        struct TimestampQueryPoolInfo {
            u32 queryCount = {}; 
            eastl::string name = {};

            PYRO_NODISCARD bool operator==(const TimestampQueryPoolInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const TimestampQueryPoolInfo&) const = default;
        };
        struct ITimestampQueryPool {
            ITimestampQueryPool() = default;
            
            /**
            * @brief Returns the query pool description.
            */
            PYRO_NODISCARD virtual const TimestampQueryPoolInfo& Info() const = 0;

            /**
            * @brief Returns a span of 64 bit timestamps that the gpu has written to. 
            * @note To make use of this, you should use DeviceLimitsInfo::nanosecondsPerTimestampTick for unit conversion.
            */
            PYRO_NODISCARD virtual eastl::span<const u64> GetTimestamps(u32 startIndex, u32 count) const = 0;

        protected:
            virtual ~ITimestampQueryPool() = default;
            friend struct IDevice;
        };

        struct OcclusionQueryPoolInfo {
            u32 queryCount = {};
            eastl::string name = {};

            PYRO_NODISCARD bool operator==(const OcclusionQueryPoolInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const OcclusionQueryPoolInfo&) const = default;
        };
        struct IOcclusionQueryPool {
            IOcclusionQueryPool() = default;

            PYRO_NODISCARD virtual const OcclusionQueryPoolInfo& Info() const = 0;
            PYRO_NODISCARD virtual eastl::span<const b32> GetResults(u32 startIndex, u32 count) const = 0;

        protected:
            virtual ~IOcclusionQueryPool() = default;
            friend struct IDevice;
        };

        struct StatisticsQueryPoolInfo {
            StatisticQueryPoolFlags flags = StatisticQueryPoolBits::NONE;
            u32 queryCount = {};
            eastl::string name = {};

            PYRO_NODISCARD bool operator==(const StatisticsQueryPoolInfo&) const = default;
            PYRO_NODISCARD bool operator!=(const StatisticsQueryPoolInfo&) const = default;
        };
        struct IStatisticsQueryPool {
            IStatisticsQueryPool() = default;

            PYRO_NODISCARD virtual const StatisticsQueryPoolInfo& Info() const = 0;
            PYRO_NODISCARD virtual eastl::span<const u32> GetResults(u32 startIndex, u32 count) const = 0;

        protected:
            virtual ~IStatisticsQueryPool() = default;
            friend struct IDevice;
        };
    } // namespace RHI
} // namespace PyroshockStudios