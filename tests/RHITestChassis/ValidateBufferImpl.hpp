#include "Helpers/ValidationFixture.hpp"
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

    TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyBuffer) {
        BufferInfo info = {};
        info.size = 1024;
        info.usage = BufferUsageFlagBits::VERTEX_BUFFER; // or however you define usage flags
        info.name = "TestBuffer";

        Buffer buffer = mDevice->CreateBuffer(info);
        ASSERT_TRUE(mDevice->IsBufferValid(buffer));

        const BufferInfo& queried = mDevice->GetBufferInfo(buffer);
        EXPECT_EQ(queried, info);

        mDevice->DestroyBuffer(buffer);
        EXPECT_FALSE(mDevice->IsBufferValid(buffer));
    }
    TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyVirtualBuffers) {
        MemoryBlockInfo blockInfo = {};
        blockInfo.size = 4096;
        blockInfo.bufferUsage = BufferUsageFlagBits::UNIFORM_BUFFER | BufferUsageFlagBits::SHADER_RESOURCE;
        blockInfo.name = "Test Memory Block";

        MemoryBlock block = mDevice->Create(blockInfo);
        ASSERT_TRUE(mDevice->IsMemoryBlockValid(block));

        eastl::vector<Buffer> buffers = {};

        for (i32 i = 0; i < 3; ++i) {
            BufferInfo info = {};
            info.size = 1024;
            info.usage = BufferUsageFlagBits::SHADER_RESOURCE;
            info.memoryBlock = block;
            info.name = "TestBuffer";

            Buffer buffer = mDevice->CreateBuffer(info);
            ASSERT_TRUE(mDevice->IsBufferValid(buffer));

            const BufferInfo& queried = mDevice->GetBufferInfo(buffer);
            EXPECT_EQ(queried, info);

            buffers.push_back(buffer);
        }

        for (i32 i = 0; i < 3; ++i) {
            mDevice->DestroyBuffer(buffers[i]);
            EXPECT_FALSE(mDevice->IsBufferValid(buffers[i]));
        }

        mDevice->DestroyMemoryBlock(block);
        EXPECT_FALSE(mDevice->IsMemoryBlockValid(block));
    }
