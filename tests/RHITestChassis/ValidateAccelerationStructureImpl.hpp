#include "Helpers/ValidationFixture.hpp"
#include <EASTL/array.h>
#include <EASTL/vector.h>
#ifdef OPAQUE
#undef OPAQUE
#endif

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

struct SimpleVertex {
    f32 x, y, z;
};

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyBlas) {
    if (!mDevice->Features().bAccelerationStructureBuild) {
        GTEST_SKIP() << "Device does not support Acceleration Structures, skipping test...";
    }
    BlasInfo info = {
        .size = 1024,
        .name = "My blas"
    };
    BlasId blas = mDevice->Create(info);
    EXPECT_EQ(mDevice->GetBlasInfo(blas), info);
    mDevice->Destroy(blas);
}
TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyTlas) {
    if (!mDevice->Features().bAccelerationStructureBuild) {
        GTEST_SKIP() << "Device does not support Acceleration Structures, skipping test...";
    }
    TlasInfo info = {
        .size = 1024,
        .name = "My tlas"
    };
    TlasId tlas = mDevice->Create(info);
    EXPECT_EQ(mDevice->GetTlasInfo(tlas), info);
    mDevice->Destroy(tlas);
}


// Test case for successfully creating a BLAS
TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateBlas_Success) {
    if (!mDevice->Features().bAccelerationStructureBuild) {
        GTEST_SKIP() << "Device does not support Acceleration Structures, skipping test...";
    }
    // --- Arrange ---
    // 1. Define geometry data
    const SimpleVertex vertices[] = {
        { 1.0f, 1.0f, 0.0f },
        { -1.0f, 1.0f, 0.0f },
        { 0.0f, -1.0f, 0.0f }
    };
    const u32 indices[] = { 0, 1, 2 };

    // 2. Create and fill Vertex Buffer
    BufferInfo vbInfo = {
        .size = sizeof(vertices),
        .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite // Mappable
    };
    Buffer vertexBuffer = mDevice->CreateBuffer(vbInfo);
    ASSERT_NE(vertexBuffer, PYRO_NULL_BUFFER);
    u8* pVertData = mDevice->BufferHostAddress(vertexBuffer);
    ASSERT_NE(pVertData, nullptr);
    memcpy(pVertData, vertices, sizeof(vertices));

    // 3. Create and fill Index Buffer
    BufferInfo ibInfo = {
        .size = sizeof(indices),
        .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite // Mappable
    };
    Buffer indexBuffer = mDevice->CreateBuffer(ibInfo);
    ASSERT_NE(indexBuffer, PYRO_NULL_BUFFER);
    u8* pIdxData = mDevice->BufferHostAddress(indexBuffer);
    ASSERT_NE(pIdxData, nullptr);
    memcpy(pIdxData, indices, sizeof(indices));

    // 4. Get size requirements
    BlasTriangleGeometryInfo triangleGeo = {
        .vertexFormat = Format::RGB32Sfloat, // 3x 32-bit floats
        .indexType = IndexType::Uint32,
        .vertexBuffer = vertexBuffer,
        .indexBuffer = indexBuffer,
        .vertexStride = sizeof(SimpleVertex),
        .vertexByteOffset = 0,
        .indexOffset = 0,
        .vertexCount = 3,
        .indexCount = 3,
        .flags = AccelerationStructureGeometryFlagBits::OPAQUE
    };
    eastl::array<BlasTriangleGeometryInfo, 1> geoArray = { triangleGeo };
    BlasBuildInfo buildInfo = { .geometries = geoArray };

    AccelerationStructureBuildSizesInfo sizeInfo = mDevice->BlasSizeRequirements(buildInfo);
    ASSERT_GT(sizeInfo.accelerationStructureSize, 0);

    // 5. Define the BLAS info based on requirements
    BlasInfo blasInfoToCreate = { .size = sizeInfo.accelerationStructureSize, .name = "Test BLAS" };

    // --- Act ---
    // 6. Create the BLAS
    BlasId blasId = mDevice->CreateBlas(blasInfoToCreate);

    // --- Assert ---
    // 7. Check if a valid ID was returned
    ASSERT_NE(blasId, PYRO_NULL_BLAS);

    // 8. Verify info (optional, but good practice)
    const auto& returnedInfo = mDevice->GetBlasInfo(blasId);
    ASSERT_EQ(returnedInfo, blasInfoToCreate);

    // --- Cleanup ---
    // 9. Destroy the BLAS and buffers
    mDevice->DestroyBlas(blasId, false);
    mDevice->DestroyBuffer(indexBuffer, false);
    mDevice->DestroyBuffer(vertexBuffer, false);
}

// Test case for successfully creating a TLAS
TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateTlas_Success) {
    if (!mDevice->Features().bAccelerationStructureBuild) {
        GTEST_SKIP() << "Device does not support Acceleration Structures, skipping test...";
    }
    // --- Arrange ---
    // 1. Create a BLAS first (required for instance data)
    // (Simplified version of the BLAS test)
    const SimpleVertex vertices[] = { { 1.f, 1.f, 0.f }, { -1.f, 1.f, 0.f }, { 0.f, -1.f, 0.f } };
    const u32 indices[] = { 0, 1, 2 };
    BufferInfo vbInfo = {
        .size = sizeof(vertices),
        .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite,
    };
    Buffer vertexBuffer = mDevice->CreateBuffer(vbInfo);
    ASSERT_NE(vertexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(vertexBuffer), vertices, sizeof(vertices));

    BufferInfo ibInfo = {
        .size = sizeof(indices),
        .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite,
    };
    Buffer indexBuffer = mDevice->CreateBuffer(ibInfo);
    ASSERT_NE(indexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(indexBuffer), indices, sizeof(indices));

    BlasTriangleGeometryInfo triangleGeo = {
        .vertexFormat = Format::RGB32Sfloat,
        .indexType = IndexType::Uint32,
        .vertexBuffer = vertexBuffer,
        .indexBuffer = indexBuffer,
        .vertexStride = sizeof(SimpleVertex),
        .vertexCount = 3,
        .indexCount = 3,
    };
    eastl::array<BlasTriangleGeometryInfo, 1> geoArray = { triangleGeo };
    BlasBuildInfo blasBuildInfo = { .geometries = geoArray };
    AccelerationStructureBuildSizesInfo blasSizeInfo = mDevice->BlasSizeRequirements(blasBuildInfo);
    ASSERT_GT(blasSizeInfo.accelerationStructureSize, 0);
    BlasInfo blasInfo = { .size = blasSizeInfo.accelerationStructureSize, .name = "TLAS's BLAS" };
    BlasId blasId = mDevice->CreateBlas(blasInfo);
    ASSERT_NE(blasId, PYRO_NULL_BLAS);
    // NOTE: In a real scenario, you would now BUILD this BLAS on a command buffer.
    // For testing CreateTlas, we assume the unbuilt BLASId is sufficient for the instance struct.

    // 2. Create Instance Buffer
    BlasInstanceData instanceData = {
        .transform = Transform::IDENTITY,
        .instanceCustomIndex = 0,
        .mask = 0xFF,
        .instanceShaderBindingTableRecordOffset = 0,
        .blasAddress = mDevice->BlasInstanceAddress(blasId), // Use the BLAS we just created
    };
    BufferInfo instanceBufferInfo = {
        .size = sizeof(instanceData),
        .usage = BufferUsageFlagBits::BLAS_INSTANCE_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite
    };
    Buffer instanceBuffer = mDevice->CreateBuffer(instanceBufferInfo);
    ASSERT_NE(instanceBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(instanceBuffer), &instanceData, sizeof(instanceData));

    // 3. Get TLAS size requirements
    TlasInstanceInfo tlasInstanceInfo = { .data = instanceBuffer, .count = 1 };
    eastl::array<TlasInstanceInfo, 1> instanceArray = { tlasInstanceInfo };
    TlasBuildInfo tlasBuildInfo = { .instances = instanceArray };

    AccelerationStructureBuildSizesInfo tlasSizeInfo = mDevice->TlasSizeRequirements(tlasBuildInfo);
    ASSERT_GT(tlasSizeInfo.accelerationStructureSize, 0);

    // 4. Define the TLAS info
    TlasInfo tlasInfoToCreate = { .size = tlasSizeInfo.accelerationStructureSize, .name = "Test TLAS" };

    // --- Act ---
    // 5. Create the TLAS
    TlasId tlasId = mDevice->CreateTlas(tlasInfoToCreate);

    // --- Assert ---
    // 6. Check if a valid ID was returned
    ASSERT_NE(tlasId, PYRO_NULL_TLAS);

    // 7. Verify info
    const auto& returnedInfo = mDevice->GetTlasInfo(tlasId);
    ASSERT_EQ(returnedInfo, tlasInfoToCreate); // Check operator==

    // --- Cleanup ---
    // 8. Destroy everything
    mDevice->DestroyTlas(tlasId, false);
    mDevice->DestroyBuffer(instanceBuffer, false);
    mDevice->DestroyBlas(blasId, false);
    mDevice->DestroyBuffer(indexBuffer, false);
    mDevice->DestroyBuffer(vertexBuffer, false);
}


// Test case for successfully BUILDING a BLAS
TEST_F(RHI_CONTEXT_FIXTURE_NAME, BuildBlas_Success) {
    if (!mDevice->Features().bAccelerationStructureBuild) {
        GTEST_SKIP() << "Device does not support Acceleration Structures, skipping test...";
    }
    // --- Arrange ---
    // 1. Setup Geometry Buffers (Same as CreateBlas_Success)
    const SimpleVertex vertices[] = { { 1.0f, 1.0f, 0.0f }, { -1.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } };
    const u32 indices[] = { 0, 1, 2 };
    BufferInfo vbInfo = {
        .size = sizeof(vertices),
        .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite,
    };
    Buffer vertexBuffer = mDevice->CreateBuffer(vbInfo);
    ASSERT_NE(vertexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(vertexBuffer), vertices, sizeof(vertices));
    BufferInfo ibInfo = {
        .size = sizeof(indices),
        .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite,
    };
    Buffer indexBuffer = mDevice->CreateBuffer(ibInfo);
    ASSERT_NE(indexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(indexBuffer), indices, sizeof(indices));

    // 2. Get Size Requirements
    BlasTriangleGeometryInfo triangleGeo = {
        .vertexFormat = Format::RGB32Sfloat,
        .indexType = IndexType::Uint32,
        .vertexBuffer = vertexBuffer,
        .indexBuffer = indexBuffer,
        .vertexStride = sizeof(SimpleVertex),
        .vertexByteOffset = 0,
        .indexOffset = 0,
        .vertexCount = 3,
        .indexCount = 3,
        .flags = AccelerationStructureGeometryFlagBits::OPAQUE
    };
    eastl::array<BlasTriangleGeometryInfo, 1> geoArray = { triangleGeo };
    BlasBuildInfo buildInfo = { .geometries = geoArray };
    AccelerationStructureBuildSizesInfo sizeInfo = mDevice->BlasSizeRequirements(buildInfo);
    ASSERT_GT(sizeInfo.accelerationStructureSize, 0);
    ASSERT_GT(sizeInfo.buildScratchSize, 0);

    // 3. Create destination BLAS object
    BlasInfo blasInfoToCreate = { .size = sizeInfo.accelerationStructureSize, .name = "Test Build BLAS" };
    BlasId blasId = mDevice->CreateBlas(blasInfoToCreate);
    ASSERT_NE(blasId, PYRO_NULL_BLAS);

    // 4. Create Scratch Buffer
    BufferInfo scratchBufferInfo = {
        .size = sizeInfo.buildScratchSize,
        .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER,
        .allocationDomain = MemoryAllocationDomain::DeviceLocal // Scratch is best on-device
    };
    Buffer scratchBuffer = mDevice->CreateBuffer(scratchBufferInfo);
    ASSERT_NE(scratchBuffer, PYRO_NULL_BUFFER);

    // 5. Get Command List
    ICommandQueue* queue = mDevice->GetCommandQueues()[0]; // Assuming first queue is valid
    ASSERT_NE(queue, nullptr);
    ICommandBuffer* cmd = queue->GetCommandBuffer({ .name = "Build BLAS commands" });
    ASSERT_NE(cmd, nullptr);

    // --- Act ---
    // 6. Record Build Command
    buildInfo.dstBlas = blasId;
    buildInfo.scratchBuffer = scratchBuffer;

    cmd->BuildAccelerationStructures({ .blasBuildInfos = eastl::span<const BlasBuildInfo>(&buildInfo, 1) });
    cmd->Complete();

    // 7. Submit and Wait
    queue->SubmitCommandBuffer(cmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle(); // Wait for the build to complete on GPU

    // --- Assert ---
    // Test passes if no validation errors were triggered by the build.
    // We can also check that the info is still correct.
    const auto& returnedInfo = mDevice->GetBlasInfo(blasId);
    ASSERT_EQ(returnedInfo, blasInfoToCreate);

    // --- Cleanup ---
    // 8. Destroy all resources
    mDevice->DestroyBlas(blasId, false);
    mDevice->DestroyBuffer(scratchBuffer, false);
    mDevice->DestroyBuffer(indexBuffer, false);
    mDevice->DestroyBuffer(vertexBuffer, false);
}

// Test case for successfully BUILDING a TLAS (and its BLAS)
TEST_F(RHI_CONTEXT_FIXTURE_NAME, BuildTlas_Success) {
    if (!mDevice->Features().bAccelerationStructureBuild) {
        GTEST_SKIP() << "Device does not support Acceleration Structures, skipping test...";
    }
    // --- Arrange ---
    // 1. Create BLAS Geometry Buffers
    const SimpleVertex vertices[] = { { 1.f, 1.f, 0.f }, { -1.f, 1.f, 0.f }, { 0.f, -1.f, 0.f } };
    const u32 indices[] = { 0, 1, 2 };
    BufferInfo vbInfo = {
        .size = sizeof(vertices),
        .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite,
    };
    Buffer vertexBuffer = mDevice->CreateBuffer(vbInfo);
    ASSERT_NE(vertexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(vertexBuffer), vertices, sizeof(vertices));
    BufferInfo ibInfo = {
        .size = sizeof(indices),
        .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite,
    };
    Buffer indexBuffer = mDevice->CreateBuffer(ibInfo);
    ASSERT_NE(indexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(indexBuffer), indices, sizeof(indices));

    // 2. Get BLAS Size Requirements
    BlasTriangleGeometryInfo triangleGeo = {
        .vertexFormat = Format::RGB32Sfloat,
        .indexType = IndexType::Uint32,
        .vertexBuffer = vertexBuffer,
        .indexBuffer = indexBuffer,
        .vertexStride = sizeof(SimpleVertex),
        .vertexCount = 3,
        .indexCount = 3,
    };
    eastl::array<BlasTriangleGeometryInfo, 1> geoArray = { triangleGeo };
    BlasBuildInfo blasBuildInfo = { .geometries = geoArray };
    AccelerationStructureBuildSizesInfo blasSizeInfo = mDevice->BlasSizeRequirements(blasBuildInfo);
    ASSERT_GT(blasSizeInfo.accelerationStructureSize, 0);
    ASSERT_GT(blasSizeInfo.buildScratchSize, 0);

    // 3. Create BLAS Object
    BlasInfo blasInfo = { .size = blasSizeInfo.accelerationStructureSize, .name = "TLAS's BLAS (for build)" };
    BlasId blasId = mDevice->CreateBlas(blasInfo);
    ASSERT_NE(blasId, PYRO_NULL_BLAS);

    // 4. Create BLAS Scratch Buffer
    BufferInfo blasScratchInfo = {
        .size = blasSizeInfo.buildScratchSize,
        .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER,
        .allocationDomain = MemoryAllocationDomain::DeviceLocal,
    };
    Buffer blasScratchBuffer = mDevice->CreateBuffer(blasScratchInfo);
    ASSERT_NE(blasScratchBuffer, PYRO_NULL_BUFFER);

    // 5. Create Instance Buffer
    BlasInstanceData instanceData = {
        .transform = Transform::IDENTITY,
        .instanceCustomIndex = 0,
        .mask = 0xFF,
        .blasAddress = mDevice->BlasInstanceAddress(blasId),
    };
    BufferInfo instanceBufferInfo = {
        .size = sizeof(instanceData),
        .usage = BufferUsageFlagBits::BLAS_INSTANCE_BUFFER,
        .initialLayout = BufferLayout::ReadOnly,
        .allocationDomain = MemoryAllocationDomain::HostRandomWrite,
    };
    Buffer instanceBuffer = mDevice->CreateBuffer(instanceBufferInfo);
    ASSERT_NE(instanceBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(instanceBuffer), &instanceData, sizeof(instanceData));

    // 6. Get TLAS Size Requirements
    TlasInstanceInfo tlasInstanceInfo = { .data = instanceBuffer, .count = 1 };
    eastl::array<TlasInstanceInfo, 1> instanceArray = { tlasInstanceInfo };
    TlasBuildInfo tlasBuildInfo = { .instances = instanceArray };
    AccelerationStructureBuildSizesInfo tlasSizeInfo = mDevice->TlasSizeRequirements(tlasBuildInfo);
    ASSERT_GT(tlasSizeInfo.accelerationStructureSize, 0);
    ASSERT_GT(tlasSizeInfo.buildScratchSize, 0);

    // 7. Create TLAS Object
    TlasInfo tlasInfoToCreate = { .size = tlasSizeInfo.accelerationStructureSize, .name = "Test Build TLAS" };
    TlasId tlasId = mDevice->CreateTlas(tlasInfoToCreate);
    ASSERT_NE(tlasId, PYRO_NULL_TLAS);

    // 8. Create TLAS Scratch Buffer
    BufferInfo tlasScratchInfo = {
        .size = tlasSizeInfo.buildScratchSize,
        .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER,
        .allocationDomain = MemoryAllocationDomain::DeviceLocal,
    };
    Buffer tlasScratchBuffer = mDevice->CreateBuffer(tlasScratchInfo);
    ASSERT_NE(tlasScratchBuffer, PYRO_NULL_BUFFER);

    // 9. Get Command List
    ICommandQueue* queue = mDevice->GetCommandQueues()[0];
    ASSERT_NE(queue, nullptr);
    ICommandBuffer* cmd = queue->GetCommandBuffer({ .name = "Build TLAS/BLAS commands" });
    ASSERT_NE(cmd, nullptr);

    // --- Act ---
    // 10. Record Build Commands (both BLAS and TLAS)
    blasBuildInfo.dstBlas = blasId;
    blasBuildInfo.scratchBuffer = blasScratchBuffer;
    tlasBuildInfo.dstTlas = tlasId;
    tlasBuildInfo.scratchBuffer = tlasScratchBuffer;

    BuildAccelerationStructuresInfo buildAllInfo = {
        .tlasBuildInfos = eastl::span<const TlasBuildInfo>(&tlasBuildInfo, 1),
        .blasBuildInfos = eastl::span<const BlasBuildInfo>(&blasBuildInfo, 1)
    };
    cmd->BuildAccelerationStructures(buildAllInfo);
    cmd->Complete();

    // 11. Submit and Wait
    queue->SubmitCommandBuffer(cmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle();

    // --- Assert ---
    // Test passes if no validation errors were triggered.
    const auto& returnedTlasInfo = mDevice->GetTlasInfo(tlasId);
    ASSERT_EQ(returnedTlasInfo, tlasInfoToCreate);
    const auto& returnedBlasInfo = mDevice->GetBlasInfo(blasId);
    ASSERT_EQ(returnedBlasInfo, blasInfo);

    // --- Cleanup ---
    // 12. Destroy everything in reverse order of creation
    mDevice->DestroyTlas(tlasId, false);
    mDevice->DestroyBuffer(tlasScratchBuffer, false);
    mDevice->DestroyBuffer(instanceBuffer, false);
    mDevice->DestroyBlas(blasId, false);
    mDevice->DestroyBuffer(blasScratchBuffer, false);
    mDevice->DestroyBuffer(indexBuffer, false);
    mDevice->DestroyBuffer(vertexBuffer, false);
}