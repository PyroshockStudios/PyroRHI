#pragma once

#include <PyroRHI/Api/Forward.hpp>
#include <PyroRHI/Api/Types.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
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
             * @brief Submits a command buffer and recycles it. The command buffer is set to NULL.
             */
            virtual void SubmitCommandBuffer(ICommandBuffer*& commandBuffer) = 0;
            /**
             * @brief Submits the swap chain and prepares it for presentation (only allowed if this is a present queue).
             */
            virtual void SubmitSwapChain(ISwapChain* swapChain) = 0;
            /**
             * @brief Waits for all commands and present operations to have complete.
             */
            virtual void WaitIdle() = 0;
            /**
             * @brief Gets the command queue description.
             */
            virtual const CommandQueueInfo& Info() const = 0;

        protected:
            virtual ~ICommandQueue() = default;
            friend struct IDevice;
        };
    } // namespace RHI
} // namespace PyroshockStudios