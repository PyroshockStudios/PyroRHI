# PyroRHI

[![Windows MSVC](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-windows-msvc.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-windows-msvc.yml)
[![Linux GCC & Clang](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-linux.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-linux.yml)
[![macOS ARM/x86](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-macos.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-macos.yml)

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

## Backends

| Backend | Platform | Status |
|---|---|---|
| Vulkan 1.3 | Windows, Linux | Active |
| DirectX 12 | Windows | Active |
| Metal | Apple platforms | In progress |

[![Vulkan 1.3 (Ubuntu GCC)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-validate-vulkan-linux.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-validate-vulkan-linux.yml)

[![DirectX 12 (Windows MSVC)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-validate-dx12-windows.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-validate-dx12-windows.yml)

## Testing

PyroRHI includes a shared validation chassis that dynamically loads each backend and runs the same API-level tests against it. This verifies the exported backend ABI as well as real behavior for resource creation, command recording, synchronization, shader compilation, and advanced feature paths.

The test suite uses GoogleTest, integrates Vulkan validation layer and DX12 debug-layer output into failures, and includes shader-driven integration tests compiled through Slang.

[![Test Coverage](https://codecov.io/gh/PyroshockStudios/PyroRHI/branch/master/graph/badge.svg)](https://codecov.io/gh/PyroshockStudios/PyroRHI)

## License

MIT. See [LICENSE](LICENSE).
