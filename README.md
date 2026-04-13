## PyroshockStudios C++ Render Hardware Interface (PyroshockStudios © 2025)

### NOTE: This is the legacy branch of PyroRHI, with different API requirements and usages.
### This branch supports GCN 1.0+ and GTX 600+ gpus, the master branch requires GTX 900+ and GCN 1.0+ graphics cards
#### Notable differences:
- Non-full bindless: Max bound UAVs is limited to 12
- No enhanced DX12 barriers (Buffer layouts + cannot transition from Undefined -> ... if the previous layout is not Undefined)

### Build Tests:
[![Windows MSVC](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-windows-msvc.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-windows-msvc.yml)
[![Linux GCC & Clang](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-linux.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-linux.yml)
[![macOS ARM/x86](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-macos.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-macos.yml)


### API Validation tests:
[![Vulkan 1.3 (Ubuntu GCC)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-validate-vulkan-linux.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-validate-vulkan-linux.yml)

[![DirectX 12 (Windows ClangCL)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-validate-dx12-windows.yml/badge.svg)](https://github.com/PyroshockStudios/PyroRHI/actions/workflows/cmake-validate-dx12-windows.yml)

[![Test Coverage](https://codecov.io/gh/PyroshockStudios/PyroRHI/branch/master/graph/badge.svg)](https://codecov.io/gh/PyroshockStudios/PyroRHI)
