# Contributing to idlgen

## Prerequisite

- cmake 3.22
- LLVM 16
- Powershell 7
- Ninja
- Visual Studio 2022
  - CMake support
- vswhere (get via `winget install vswhere`)

#### For Building Clang

- Python3

## Getting Started

1. Run the following
```
./scripts/clone-llvm
./scripts/populate-build-ninja -config Debug
./scripts/build-clang -config Debug
```
Current script always build for `x64` on `x64` host.

2. Open `dev` with VS and starts building `x64-Debug`. Or, run `./scripts/build-idlgen`.
3. Test by `./scripts/test -config Debug -gen stdout`. Inspect the output manually.
4. Or, test in sample app. See [sample-app/README.md](sample-app/README.md)
