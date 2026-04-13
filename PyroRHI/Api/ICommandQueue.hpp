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

#include <PyroRHI/Api/Forward.hpp>
#include <PyroRHI/Api/Types.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        struct CommandBufferInfo;
        /**
         * @brief Create info struct describing how the command queue should be created.
         */
        struct CommandQueueInfo {
            /**
             * @brief Sets which command operations should be supported
             */
            CommandQueueFlags flags = {};
            /**
             * @brief The priority of the command queue scheduler
             */
            float priority = 0.0f;
            /**
             * @brief Should the command queue support swap chain presenting?
             */
            bool bPresentable = false;
            /**
             * @brief Optional debug name.
             */
            eastl::string name = {};
        };

        /**
         * @brief Interface for submitting GPU commands and swap chains (if allowed).
         */
        struct ICommandQueue {
            ICommandQueue() = default;

            /**
             * @brief Gets a brand new command buffer tied to this command queue. 
             * @note Command buffers must ALWAYS be submitted back to the queue they came from.
             */
            virtual ICommandBuffer* GetCommandBuffer(const CommandBufferInfo& info) = 0;
            /**
             * @brief Waits for all commands and present operations to have complete.
             */
            virtual void WaitIdle() = 0;
            /**
             * @brief Gets the command queue description.
             */
            virtual const CommandQueueInfo& Info() const = 0;

            /**
             * @brief Gets the nanoseconds per tick of timestamp queries in this command queue.
             */
            virtual f64 GetTimestampTickPeriodNs() const = 0;
        protected:
            virtual ~ICommandQueue() = default;
            friend struct IDevice;
        };
    } // namespace RHI
} // namespace PyroshockStudios