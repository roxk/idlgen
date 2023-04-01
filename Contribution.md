# Contributing to idlgen

## Prerequisite

- cmake 3.22
- LLVM 16
- Powershell 7
- Ninja
- Visual Studio 2022
  - CMake support

#### For Building Clang

- Python3

## Getting Started

1. Clone llvm-project to root folder.
2. Run
```
./scripts/populate-build-ninja.ps1
# After cmake is finished
./scripts/build-clang.ps1
```
Current script always build for `x64` on `x64` host.

3. Open `dev` with VS and starts building.