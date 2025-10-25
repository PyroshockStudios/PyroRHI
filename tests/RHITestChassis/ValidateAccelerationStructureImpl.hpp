#include "Helpers/ValidationFixture.hpp"
#include <EASTL/array.h>
#include <EASTL/vector.h>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyBLAS) {
    BLASInfo info = {
        .flags = BLASCreateFlagBits::NONE,
        .name = "My BLAS"
    };
    BLASId blas = mDevice->Create(info);
    EXPECT_EQ(mDevice->GetBLASInfo(blas), info);
    mDevice->Destroy(blas);
}
TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyTLAS) {
    TLASInfo info = {
        .flags = TLASCreateFlagBits::NONE,
        .name = "My TLAS"
    };
    TLASId tlas = mDevice->Create(info);
    EXPECT_EQ(mDevice->GetTLASInfo(tlas), info);
    mDevice->Destroy(tlas);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, BuildBLAS) {
    ICommandQueue* queue = mDevice->GetCommandQueues()[0];

    // 1. Create BLAS
    BLASInfo info = {
        .flags = BLASCreateFlagBits::PREFER_FAST_BUILD, 
        .name = "My BLAS"
    };
    BLASId blas = mDevice->Create(info);

    // 2. Create vertex buffer (single triangle)
    struct Vertex { float x, y, z; };
    eastl::array<Vertex, 3> vertices = {
        Vertex{0.0f, 0.0f, 0.0f},
        Vertex{1.0f, 0.0f, 0.0f},
        Vertex{0.0f, 1.0f, 0.0f}
    };

    BufferInfo vertexBufferInfo{};
    vertexBufferInfo.size = sizeof(vertices);
    vertexBufferInfo.usage = BufferUsageFlagBits::VERTEX_BUFFER | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER;
    vertexBufferInfo.allocationDomain = MemoryAllocationDomain::HostRandomWrite;
    vertexBufferInfo.name = "BLAS Vertex Buffer";

    Buffer vertexBuffer = mDevice->CreateBuffer(vertexBufferInfo);
    memcpy(mDevice->BufferHostAddress(vertexBuffer), vertices.data(), sizeof(vertices));

    // 3. Create index buffer
    eastl::array<u32, 3> indices = { 0, 1, 2 };
    BufferInfo indexBufferInfo{};
    indexBufferInfo.size = sizeof(indices);
    indexBufferInfo.usage = BufferUsageFlagBits::INDEX_BUFFER | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER;
    indexBufferInfo.allocationDomain = MemoryAllocationDomain::HostRandomWrite;
    indexBufferInfo.name = "BLAS Index Buffer";

    Buffer indexBuffer = mDevice->CreateBuffer(indexBufferInfo);
    memcpy(mDevice->BufferHostAddress(indexBuffer), indices.data(), sizeof(indices));

    // 4. Prepare geometry info
    eastl::vector<BLASGeometryInfo> geometries = {
        BLASGeometryInfo{
            .type = BLASGeometryType::Triangles,
            .vertexFormat = Format::RGB32Sfloat,
            .indexType = IndexType::Uint32,
            .vertexBuffer = vertexBuffer,
            .indexBuffer  = indexBuffer,
            .vertexStride = sizeof(Vertex),
            .vertexCount  = static_cast<u32>(vertices.size()),
            .indexCount   = static_cast<u32>(indices.size()),
            .opacity      = BLASGeometryOpacity::Opaque
        }
    };

    u32 primitiveCount = static_cast<u32>(indices.size()) / 3;

    // 5. Query required scratch buffer size
    DeviceSize reqSize = mDevice->BLASBuildSizeRequirements(blas, geometries, primitiveCount);

    // 6. Allocate scratch buffer
    BufferInfo scratchBufferInfo{};
    scratchBufferInfo.size = reqSize;
    scratchBufferInfo.usage = BufferUsageFlagBits::TLAS_BLAS_SCRATCH_BUFFER;
    scratchBufferInfo.name = "BLAS Scratch Buffer";

    Buffer scratchBuffer = mDevice->CreateBuffer(scratchBufferInfo);

    // 7. Build BLAS
    BuildBLASInfo buildInfo{};
    buildInfo.scratchBuffer = scratchBuffer;
    buildInfo.geometries = geometries;

    ICommandBuffer* cmd = queue->GetCommandBuffer({ .name = "AS Build Commands" });
    cmd->BuildBLAS(buildInfo);
    cmd->Complete();

    // 8. Submit and wait
    queue->SubmitCommandBuffer(cmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle();

    // 9. Cleanup
    mDevice->Destroy(blas);
    mDevice->Destroy(vertexBuffer);
    mDevice->Destroy(indexBuffer);
    mDevice->Destroy(scratchBuffer);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, BuildTLAS) {
    ICommandQueue* queue = mDevice->GetCommandQueues()[0];

    // --- 1. Create a BLAS first ---
    BLASInfo blasInfo = {
        .flags = BLASCreateFlagBits::ALLOW_REBUILD,
        .name = "My BLAS for TLAS"
    };
    BLAS blas = mDevice->Create(blasInfo);

    // Minimal geometry: a single triangle
    struct Vertex { float x, y, z; };
    eastl::array<Vertex, 3> vertices = {
        Vertex{0.0f, 0.0f, 0.0f},
        Vertex{1.0f, 0.0f, 0.0f},
        Vertex{0.0f, 1.0f, 0.0f}
    };
    eastl::array<u32, 3> indices = { 0, 1, 2 };

   
    BufferInfo vertexBufferInfo{};
    vertexBufferInfo.size = sizeof(vertices);
    vertexBufferInfo.usage = BufferUsageFlagBits::VERTEX_BUFFER | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER;
    vertexBufferInfo.allocationDomain = MemoryAllocationDomain::HostRandomWrite;
    vertexBufferInfo.name = "BLAS Vertex Buffer";

    Buffer vertexBuffer = mDevice->CreateBuffer(vertexBufferInfo);
    memcpy(mDevice->BufferHostAddress(vertexBuffer), vertices.data(), sizeof(vertices));

    // 3. Create index buffer
    eastl::array<u32, 3> indices = { 0, 1, 2 };
    BufferInfo indexBufferInfo{};
    indexBufferInfo.size = sizeof(indices);
    indexBufferInfo.usage = BufferUsageFlagBits::INDEX_BUFFER | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER;
    indexBufferInfo.allocationDomain = MemoryAllocationDomain::HostRandomWrite;
    indexBufferInfo.name = "BLAS Index Buffer";

    Buffer indexBuffer = mDevice->CreateBuffer(indexBufferInfo);
    memcpy(mDevice->BufferHostAddress(indexBuffer), indices.data(), sizeof(indices));

    eastl::vector<BLASGeometryInfo> geometries = {
        BLASGeometryInfo{
            .type = BLASGeometryType::Triangles,
            .vertexFormat = Format::RGB32Sfloat,
            .indexType = IndexType::Uint32,
            .vertexBuffer = vertexBuffer,
            .indexBuffer  = indexBuffer,
            .vertexStride = sizeof(Vertex),
            .vertexCount  = static_cast<u32>(vertices.size()),
            .indexCount   = static_cast<u32>(indices.size()),
            .opacity      = BLASGeometryOpacity::Opaque
        }
    };

    u32 primitiveCount = static_cast<u32>(indices.size()) / 3;
    DeviceSize blasScratchSize = mDevice->BLASBuildSizeRequirements(blas, geometries, primitiveCount);

    // 6. Allocate scratch buffer
    BufferInfo scratchBufferInfo{};
    scratchBufferInfo.size = blasScratchSize;
    scratchBufferInfo.usage = BufferUsageFlagBits::TLAS_BLAS_SCRATCH_BUFFER;
    scratchBufferInfo.name = "BLAS Scratch Buffer";

    Buffer blasScratchBuffer = mDevice->CreateBuffer(scratchBufferInfo);

    // 7. Build BLAS
    BuildBLASInfo buildBLASInfo{};
    buildBLASInfo.scratchBuffer = blasScratchBuffer;
    buildBLASInfo.geometries = geometries;


    ICommandBuffer* cmd = queue->GetCommandBuffer({ .name = "BLAS Build Commands" });
    cmd->BuildBLAS(buildBLASInfo);
    cmd->Complete();
    queue->SubmitCommandBuffer(cmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle();

    // --- 2. Create TLAS ---
    TLASInfo tlasInfo{
        .flags = TLASCreateFlagBits::PREFER_FAST_BUILD,
        .name = "My TLAS"
    };
    TLASId tlas = mDevice->Create(tlasInfo);

    // --- 3. Create TLAS instance referencing the BLAS ---
    TLASInstanceInfo instance{};
    instance.blas = blas;
    instance.transform = Transform::IDENTITY;
    instance.instanceID = 1;
    instance.hitGroupMask = 0xFFFFFFFF;
    instance.opacity = TLASInstanceOpacity::Opaque;

    eastl::array<TLASInstanceInfo, 1> instances = { instance };
    u32 instanceCount = static_cast<u32>(instances.size());

    DeviceSize tlasScratchSize = mDevice->TLASBuildSizeRequirements(tlas, instances, instanceCount);
        // 6. Allocate scratch buffer
    BufferInfo tlasScratchBufferInfo{};
    tlasScratchBufferInfo.size = tlasScratchSize;
    tlasScratchBufferInfo.usage = BufferUsageFlagBits::TLAS_BLAS_SCRATCH_BUFFER;
    tlasScratchBufferInfo.name = "TLAS Scratch Buffer";

    Buffer tlasScratchBuffer = mDevice->CreateBuffer(tlasScratchBufferInfo);

    // --- 4. Build TLAS ---
    BuildTLASInfo buildTLASInfo{};
    buildTLASInfo.scratchBuffer = tlasScratchBuffer;
    buildTLASInfo.instances = instances;

    ICommandBuffer* tlasCmd = queue->GetCommandBuffer({ .name = "TLAS Build Commands" });
    tlasCmd->BuildTLAS(buildTLASInfo);
    tlasCmd->Complete();

    queue->SubmitCommandBuffer(tlasCmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle();

    // --- 5. Cleanup ---
    mDevice->Destroy(tlas);
    mDevice->Destroy(blas);
    mDevice->Destroy(vertexBuffer);
    mDevice->Destroy(indexBuffer);
    mDevice->Destroy(blasScratchBuffer);
    mDevice->Destroy(tlasScratchBuffer);
}
