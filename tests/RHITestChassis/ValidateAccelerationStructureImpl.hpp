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
    TlasBuildInfo tlasBuildInfo = { .instances = tlasInstanceInfo };

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
    TlasBuildInfo tlasBuildInfo = { .instances = tlasInstanceInfo };
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


// Test case for successfully UPDATING a BLAS
TEST_F(RHI_CONTEXT_FIXTURE_NAME, UpdateBlas_Success) {
    if (!mDevice->Features().bAccelerationStructureBuild) {
        GTEST_SKIP() << "Device does not support Acceleration Structures, skipping test...";
    }
    // --- Arrange ---
    // 1. Setup *Initial* Geometry Buffers
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

    // 2. Get Size Requirements for *initial build* (must allow update)
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
    BlasBuildInfo initialBuildInfo = {
        .flags = AccelerationStructureCreateFlagBits::ALLOW_UPDATE | AccelerationStructureCreateFlagBits::PREFER_FAST_BUILD,
        .geometries = geoArray
    };
    AccelerationStructureBuildSizesInfo sizeInfo = mDevice->BlasSizeRequirements(initialBuildInfo);
    ASSERT_GT(sizeInfo.accelerationStructureSize, 0);
    ASSERT_GT(sizeInfo.buildScratchSize, 0);
    ASSERT_GT(sizeInfo.updateScratchSize, 0); // Need this for the update

    // 3. Create destination BLAS object
    BlasInfo blasInfoToCreate = { .size = sizeInfo.accelerationStructureSize, .name = "Test Update BLAS" };
    BlasId blasId = mDevice->CreateBlas(blasInfoToCreate);
    ASSERT_NE(blasId, PYRO_NULL_BLAS);

    // 4. Create Scratch Buffers (one for build, one for update)
    BufferInfo buildScratchInfo = {
        .size = sizeInfo.buildScratchSize,
        .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER,
        .allocationDomain = MemoryAllocationDomain::DeviceLocal
    };
    Buffer buildScratchBuffer = mDevice->CreateBuffer(buildScratchInfo);
    ASSERT_NE(buildScratchBuffer, PYRO_NULL_BUFFER);

    BufferInfo updateScratchInfo = {
        .size = sizeInfo.updateScratchSize,
        .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER,
        .allocationDomain = MemoryAllocationDomain::DeviceLocal
    };
    Buffer updateScratchBuffer = mDevice->CreateBuffer(updateScratchInfo);
    ASSERT_NE(updateScratchBuffer, PYRO_NULL_BUFFER);

    // 5. Get Command List and perform *initial build*
    ICommandQueue* queue = mDevice->GetCommandQueues()[0];
    ASSERT_NE(queue, nullptr);
    ICommandBuffer* cmd = queue->GetCommandBuffer({ .name = "Initial BLAS Build commands" });
    ASSERT_NE(cmd, nullptr);

    initialBuildInfo.dstBlas = blasId;
    initialBuildInfo.scratchBuffer = buildScratchBuffer;
    cmd->BuildAccelerationStructures({ .blasBuildInfos = eastl::span<const BlasBuildInfo>(&initialBuildInfo, 1) });
    cmd->Complete();
    queue->SubmitCommandBuffer(cmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle(); // Wait for the initial build

    // --- Act ---
    // 6. Setup *Updated* Geometry Buffers (e.g., move the triangle)
    const SimpleVertex updatedVertices[] = { { 2.0f, 2.0f, 0.0f }, { 0.0f, 2.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } };
    BufferInfo updatedVbInfo = vbInfo; // Copy info
    Buffer updatedVertexBuffer = mDevice->CreateBuffer(updatedVbInfo);
    ASSERT_NE(updatedVertexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(updatedVertexBuffer), updatedVertices, sizeof(updatedVertices));

    // 7. Define Update Info
    BlasTriangleGeometryInfo updatedTriangleGeo = triangleGeo; // Copy base info
    updatedTriangleGeo.vertexBuffer = updatedVertexBuffer;     // Point to new data
    eastl::array<BlasTriangleGeometryInfo, 1> updatedGeoArray = { updatedTriangleGeo };

    BlasBuildInfo updateBuildInfo = {
        .flags = AccelerationStructureCreateFlagBits::PREFER_FAST_BUILD, // Flags for the *operation*
        .bUpdate = true,
        .srcBlas = blasId,
        .dstBlas = blasId,
        .geometries = updatedGeoArray,
        .scratchBuffer = updateScratchBuffer
    };

    // 8. Record Update Command
    ICommandBuffer* updateCmd = queue->GetCommandBuffer({ .name = "Update BLAS commands" });
    ASSERT_NE(updateCmd, nullptr);
    updateCmd->BuildAccelerationStructures({ .blasBuildInfos = eastl::span<const BlasBuildInfo>(&updateBuildInfo, 1) });
    updateCmd->Complete();

    // 9. Submit and Wait
    queue->SubmitCommandBuffer(updateCmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle();

    // --- Assert ---
    // Test passes if no validation errors were triggered.
    const auto& returnedInfo = mDevice->GetBlasInfo(blasId);
    ASSERT_EQ(returnedInfo, blasInfoToCreate);

    // --- Cleanup ---
    // 10. Destroy all resources
    mDevice->DestroyBlas(blasId, false);
    mDevice->DestroyBuffer(buildScratchBuffer, false);
    mDevice->DestroyBuffer(updateScratchBuffer, false);
    mDevice->DestroyBuffer(indexBuffer, false);
    mDevice->DestroyBuffer(vertexBuffer, false);
    mDevice->DestroyBuffer(updatedVertexBuffer, false);
}

// Test case for successfully UPDATING a TLAS
TEST_F(RHI_CONTEXT_FIXTURE_NAME, UpdateTlas_Success) {
    if (!mDevice->Features().bAccelerationStructureBuild) {
        GTEST_SKIP() << "Device does not support Acceleration Structures, skipping test...";
    }
    // --- Arrange ---
    // 1. Build a BLAS (re-using setup from BuildTlas_Success)
    const SimpleVertex vertices[] = { { 1.f, 1.f, 0.f }, { -1.f, 1.f, 0.f }, { 0.f, -1.f, 0.f } };
    const u32 indices[] = { 0, 1, 2 };
    BufferInfo vbInfo = { .size = sizeof(vertices), .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER, .initialLayout = BufferLayout::ReadOnly, .allocationDomain = MemoryAllocationDomain::HostRandomWrite };
    Buffer vertexBuffer = mDevice->CreateBuffer(vbInfo);
    ASSERT_NE(vertexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(vertexBuffer), vertices, sizeof(vertices));
    BufferInfo ibInfo = { .size = sizeof(indices), .usage = BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER, .initialLayout = BufferLayout::ReadOnly, .allocationDomain = MemoryAllocationDomain::HostRandomWrite };
    Buffer indexBuffer = mDevice->CreateBuffer(ibInfo);
    ASSERT_NE(indexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(indexBuffer), indices, sizeof(indices));
    BlasTriangleGeometryInfo triangleGeo = { .vertexFormat = Format::RGB32Sfloat, .indexType = IndexType::Uint32, .vertexBuffer = vertexBuffer, .indexBuffer = indexBuffer, .vertexStride = sizeof(SimpleVertex), .vertexCount = 3, .indexCount = 3 };
    eastl::array<BlasTriangleGeometryInfo, 1> geoArray = { triangleGeo };
    BlasBuildInfo blasBuildInfo = { .geometries = geoArray };
    AccelerationStructureBuildSizesInfo blasSizeInfo = mDevice->BlasSizeRequirements(blasBuildInfo);
    ASSERT_GT(blasSizeInfo.accelerationStructureSize, 0);
    ASSERT_GT(blasSizeInfo.buildScratchSize, 0);
    BlasInfo blasInfo = { .size = blasSizeInfo.accelerationStructureSize, .name = "TLAS's BLAS (for update test)" };
    BlasId blasId = mDevice->CreateBlas(blasInfo);
    ASSERT_NE(blasId, PYRO_NULL_BLAS);
    BufferInfo blasScratchInfo = { .size = blasSizeInfo.buildScratchSize, .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER, .allocationDomain = MemoryAllocationDomain::DeviceLocal };
    Buffer blasScratchBuffer = mDevice->CreateBuffer(blasScratchInfo);
    ASSERT_NE(blasScratchBuffer, PYRO_NULL_BUFFER);
    blasBuildInfo.dstBlas = blasId;
    blasBuildInfo.scratchBuffer = blasScratchBuffer;

    // 2. Create *Initial* Instance Buffer
    BlasInstanceData instanceData = {
        .transform = Transform::IDENTITY, // Initial position
        .instanceCustomIndex = 0,
        .mask = 0xFF,
        .blasAddress = mDevice->BlasInstanceAddress(blasId),
    };
    BufferInfo instanceBufferInfo = { .size = sizeof(instanceData), .usage = BufferUsageFlagBits::BLAS_INSTANCE_BUFFER, .initialLayout = BufferLayout::ReadOnly, .allocationDomain = MemoryAllocationDomain::HostRandomWrite };
    Buffer instanceBuffer = mDevice->CreateBuffer(instanceBufferInfo);
    ASSERT_NE(instanceBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(instanceBuffer), &instanceData, sizeof(instanceData));

    // 3. Get TLAS Size Requirements (must allow update)
    TlasInstanceInfo tlasInstanceInfo = { .data = instanceBuffer, .count = 1 };
    TlasBuildInfo initialBuildInfo = {
        .flags = AccelerationStructureCreateFlagBits::ALLOW_UPDATE | AccelerationStructureCreateFlagBits::PREFER_FAST_BUILD,
        .instances = tlasInstanceInfo
    };
    AccelerationStructureBuildSizesInfo tlasSizeInfo = mDevice->TlasSizeRequirements(initialBuildInfo);
    ASSERT_GT(tlasSizeInfo.accelerationStructureSize, 0);
    ASSERT_GT(tlasSizeInfo.buildScratchSize, 0);
    ASSERT_GT(tlasSizeInfo.updateScratchSize, 0); // Need this

    // 4. Create TLAS Object
    TlasInfo tlasInfoToCreate = { .size = tlasSizeInfo.accelerationStructureSize, .name = "Test Update TLAS" };
    TlasId tlasId = mDevice->CreateTlas(tlasInfoToCreate);
    ASSERT_NE(tlasId, PYRO_NULL_TLAS);

    // 5. Create TLAS Scratch Buffers
    BufferInfo tlasBuildScratchInfo = { .size = tlasSizeInfo.buildScratchSize, .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER, .allocationDomain = MemoryAllocationDomain::DeviceLocal, };
    Buffer tlasBuildScratchBuffer = mDevice->CreateBuffer(tlasBuildScratchInfo);
    ASSERT_NE(tlasBuildScratchBuffer, PYRO_NULL_BUFFER);

    BufferInfo tlasUpdateScratchInfo = { .size = tlasSizeInfo.updateScratchSize, .usage = BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER, .allocationDomain = MemoryAllocationDomain::DeviceLocal, };
    Buffer tlasUpdateScratchBuffer = mDevice->CreateBuffer(tlasUpdateScratchInfo);
    ASSERT_NE(tlasUpdateScratchBuffer, PYRO_NULL_BUFFER);

    // 6. Get Command List and perform *initial build* (BLAS + TLAS)
    ICommandQueue* queue = mDevice->GetCommandQueues()[0];
    ASSERT_NE(queue, nullptr);
    ICommandBuffer* cmd = queue->GetCommandBuffer({ .name = "Initial TLAS/BLAS Build" });
    ASSERT_NE(cmd, nullptr);

    initialBuildInfo.dstTlas = tlasId;
    initialBuildInfo.scratchBuffer = tlasBuildScratchBuffer;
    BuildAccelerationStructuresInfo buildAllInfo = {
        .tlasBuildInfos = eastl::span<const TlasBuildInfo>(&initialBuildInfo, 1),
        .blasBuildInfos = eastl::span<const BlasBuildInfo>(&blasBuildInfo, 1) // Build the BLAS
    };
    cmd->BuildAccelerationStructures(buildAllInfo);
    cmd->Complete();
    queue->SubmitCommandBuffer(cmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle();

    // --- Act ---
    // 7. Create *Updated* Instance Buffer (e.g., move the instance)
    BlasInstanceData updatedInstanceData = instanceData;
    updatedInstanceData.transform = Transform(1.0f, 0.0f, 0.0f, 10.0f, // Translate 10 units on X
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f);

    BufferInfo updatedInstanceBufferInfo = instanceBufferInfo; // Copy info
    Buffer updatedInstanceBuffer = mDevice->CreateBuffer(updatedInstanceBufferInfo);
    ASSERT_NE(updatedInstanceBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(updatedInstanceBuffer), &updatedInstanceData, sizeof(updatedInstanceData));

    // 8. Define Update Info
    TlasInstanceInfo updatedTlasInstanceInfo = { .data = updatedInstanceBuffer, .count = 1 };

    TlasBuildInfo updateBuildInfo = {
        .flags = AccelerationStructureCreateFlagBits::PREFER_FAST_BUILD,
        .update = true,
        .srcTlas = tlasId,
        .dstTlas = tlasId,
        .instances = updatedTlasInstanceInfo,
        .scratchBuffer = tlasUpdateScratchBuffer,
    };

    // 9. Record Update Command
    ICommandBuffer* updateCmd = queue->GetCommandBuffer({ .name = "Update TLAS commands" });
    ASSERT_NE(updateCmd, nullptr);
    updateCmd->BuildAccelerationStructures({ .tlasBuildInfos = eastl::span<const TlasBuildInfo>(&updateBuildInfo, 1) });
    updateCmd->Complete();

    // 10. Submit and Wait
    queue->SubmitCommandBuffer(updateCmd);
    mDevice->SubmitQueue({ .queue = queue });
    mDevice->WaitIdle();

    // --- Assert ---
    // Test passes if no validation errors were triggered.
    const auto& returnedTlasInfo = mDevice->GetTlasInfo(tlasId);
    ASSERT_EQ(returnedTlasInfo, tlasInfoToCreate);

    // --- Cleanup ---
    // 11. Destroy all resources
    mDevice->DestroyTlas(tlasId, false);
    mDevice->DestroyBuffer(tlasBuildScratchBuffer, false);
    mDevice->DestroyBuffer(tlasUpdateScratchBuffer, false);
    mDevice->DestroyBuffer(instanceBuffer, false);
    mDevice->DestroyBuffer(updatedInstanceBuffer, false);
    mDevice->DestroyBlas(blasId, false);
    mDevice->DestroyBuffer(blasScratchBuffer, false);
    mDevice->DestroyBuffer(indexBuffer, false);
    mDevice->DestroyBuffer(vertexBuffer, false);
}
