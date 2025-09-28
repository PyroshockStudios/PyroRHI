#pragma once
#include <RHIDX12/Core.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        /**
         * @brief A linear buffer allocator for uploading data to the GPU using a single persistent D3D12 upload heap.
         *
         * This class manages a large persistent-mapped upload buffer as a linear buffer.
         * Each allocation sub-allocates a chunk from the buffer. If the end if reached, another buffer must be allocated.
         *
         * This is mostly used for emulating Vulkan's vkCmdUpdateBuffer
         *
         * Thread-safety:
         * - Not thread-safe; expected to be called from a single-threaded command recording context.
         *
         */
        class LinearUploadBuffer : DeleteCopy, DeleteMove {
        public:
            /// Alignment requirement for buffer allocations (256 bytes for D3D12 constant buffer compatibility).
            static constexpr UINT64 alignment = 256;

            /**
             * @brief Creates a linear buffer with a single large D3D12 UPLOAD heap.
             *
             * @param device       Pointer to the D3D12 device used to allocate the buffer.
             * @param bufferSize   Total size of the ring buffer in bytes.
             *
             * This allocates one persistent-mapped buffer in the D3D12 upload heap.
             */
            LinearUploadBuffer(ID3D12Device* device, UINT64 bufferSize);
            /**
             * @brief Destructor. Unmaps the persistent upload buffer.
             */
            ~LinearUploadBuffer();

            /**
             * @brief Allocates a slice of the ring buffer for uploading data.
             *
             * @param size        Size of the requested allocation in bytes. Will be aligned to @ref alignment.
             * @param outOffset   Offset into the ring buffer resource where the allocation starts.
             *
             * @return Pointer to CPU-visible memory for writing upload data,
             *         or nullptr if allocation would overwrite an invalid range.
             *
             * The ring buffer wraps around when reaching the end. If the allocation would overlap with
             * invalidated ranges, nullptr is returned.
             */
            void* Allocate(UINT64 size, UINT64& outOffset);

            /**
             * @brief Returns the underlying D3D12 upload resource.
             */
            ID3D12Resource* GetResource() const { return mUploadBuffer.Get(); }

            /**
             * @brief Resets the offset back to 0
             */
            void Reset() { mOffset = 0; }

        private:
            ComPtr<ID3D12Resource> mUploadBuffer = {}; ///< D3D12 upload heap resource.
            BYTE* mMappedPtr = nullptr;                ///< CPU-visible pointer to the mapped buffer.
            UINT64 mSize = 0;                          ///< Total size of the buffer in bytes.
            UINT64 mOffset = 0;                        ///< Current allocation offset.
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios