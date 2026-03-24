# PyroRHI

> Modern C++ Rendering Hardware Interface

PyroRHI is a cross-platform C++ abstraction layer over explicit graphics APIs. It exposes a shared interface for GPU devices, command buffers, queues, pipelines, resource views, swap chains, synchronization primitives, and acceleration structures, with backend implementations for Vulkan and DirectX 12.

The goal is not to hide low-level rendering, but to present it through a consistent API that preserves modern concepts such as explicit barriers, multi-queue execution, bindless-style resource access, and capability-driven feature selection.

## Features

- Unified RHI API for Vulkan and DirectX 12 backends
- Explicit synchronization and resource state transitions
- Buffers, images, samplers, render targets, swap chains, fences, semaphores, queries, and pipelines
- Optional memory block suballocation for engine-style resource management
- Shader model and backend feature queries
- Modern GPU features including acceleration structures and ray-query-oriented workflows where supported

## Testing

PyroRHI includes a shared validation chassis that dynamically loads each backend and runs the same API-level tests against it. This verifies the exported backend ABI as well as real behavior for resource creation, command recording, synchronization, shader compilation, and advanced feature paths.

The test suite uses GoogleTest, integrates Vulkan validation layer and DX12 debug-layer output into failures, and includes shader-driven integration tests compiled through Slang.

## Backends

| Backend | Platform | Status |
|---|---|---|
| Vulkan 1.3 | Windows, Linux | Active |
| DirectX 12 | Windows | Active |
| Metal | Apple platforms | In progress |

## License

MIT. See [LICENSE](LICENSE).
