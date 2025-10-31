#include "Helpers/ValidationFixture.hpp"
using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyMemoryBlock) {
    MemoryBlockInfo blockInfo = {};
    blockInfo.size = 1024;
    blockInfo.bufferUsage = BufferUsageFlagBits::UNIFORM_BUFFER | BufferUsageFlagBits::SHADER_RESOURCE;
    blockInfo.name = "Test Memory Block";

    MemoryBlock block = mDevice->CreateMemoryBlock(blockInfo);
    ASSERT_TRUE(mDevice->IsMemoryBlockValid(block));

    const MemoryBlockInfo& queried = mDevice->GetMemoryBlockInfo(block);
    EXPECT_EQ(queried, info);

    mDevice->DestroyMemoryBlock(block);
    EXPECT_FALSE(mDevice->IsMemoryBlockValid(block));
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyMemoryBlockOverload) {
    MemoryBlockInfo blockInfo = {};
    blockInfo.size = 1024;
    blockInfo.bufferUsage = BufferUsageFlagBits::UNIFORM_BUFFER | BufferUsageFlagBits::SHADER_RESOURCE;
    blockInfo.name = "Test Memory Block";

    MemoryBlock block = mDevice->Create(blockInfo);
    ASSERT_TRUE(mDevice->IsMemoryBlockValid(block));

    const MemoryBlockInfo& queried = mDevice->GetMemoryBlockInfo(block);
    EXPECT_EQ(queried, info);

    mDevice->Destroy(block);
    EXPECT_FALSE(mDevice->IsValid(block));
}
