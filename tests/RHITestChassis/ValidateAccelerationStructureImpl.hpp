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
        .usage = BufferUsageFlagBits::VERTEX_BUFFER | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER | BufferUsageFlagBits::BUFFER_DEVICE_ADDRESS,
        .initialLayout = BufferLayout::Undefined,
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
        .usage = BufferUsageFlagBits::INDEX_BUFFER | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER | BufferUsageFlagBits::BUFFER_DEVICE_ADDRESS,
        .initialLayout = BufferLayout::Undefined,
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
        .vertexOffset = 0,
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
    ASSERT_EQ(returnedInfo.size, blasInfoToCreate.size);
    ASSERT_EQ(returnedInfo.name, blasInfoToCreate.name);

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
    BufferInfo vbInfo = { .size = sizeof(vertices), .usage = BufferUsageFlagBits::VERTEX_BUFFER | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER | BufferUsageFlagBits::BUFFER_DEVICE_ADDRESS, .allocationDomain = MemoryAllocationDomain::HostRandomWrite };
    Buffer vertexBuffer = mDevice->CreateBuffer(vbInfo);
    ASSERT_NE(vertexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(vertexBuffer), vertices, sizeof(vertices));

    BufferInfo ibInfo = { .size = sizeof(indices), .usage = BufferUsageFlagBits::INDEX_BUFFER | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER | BufferUsageFlagBits::BUFFER_DEVICE_ADDRESS, .allocationDomain = MemoryAllocationDomain::HostRandomWrite };
    Buffer indexBuffer = mDevice->CreateBuffer(ibInfo);
    ASSERT_NE(indexBuffer, PYRO_NULL_BUFFER);
    memcpy(mDevice->BufferHostAddress(indexBuffer), indices, sizeof(indices));

    BlasTriangleGeometryInfo triangleGeo = { .vertexFormat = Format::RGB32Sfloat, .indexType = IndexType::Uint32, .vertexBuffer = vertexBuffer, .indexBuffer = indexBuffer, .vertexStride = sizeof(SimpleVertex), .vertexCount = 3, .indexCount = 3 };
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
        .blas = blasId, // Use the ID of the BLAS we just created
        .transform = Transform::IDENTITY,
        .instanceCustomIndex = 0,
        .mask = 0xFF,
        .instanceShaderBindingTableRecordOffset = 0,
        .flags = 0 // Assuming 0 is valid
    };
    BufferInfo instanceBufferInfo = {
        .size = sizeof(instanceData),
        .usage = BufferUsageFlagBits::SHADER_RESOURCE | BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER | BufferUsageFlagBits::BUFFER_DEVICE_ADDRESS,
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
