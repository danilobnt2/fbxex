# Contributing

## Contribution flow and CI/CD

- Work happens on the `develop` branch. Please contribute via fork -> PR targeting `develop`.
- CI runs on every push/PR and builds/tests both runtime and UI, including coverage gates.
- New releases are managed by maintainers via PRs from `develop` to `main`
- SemVer bump markers are required on PRs targeting `main` (`semver:major|minor|patch` label or `[major]/[minor]/[patch]` in the title).
- When a SemVer tag is pushed on `main`, CD builds the app with the tag version, packages the release zip, and publishes a GitHub release with PR titles merged into `develop` since the previous tag.

## Building from Source

### Prerequisites

- [Git](https://gitforwindows.org/)
- [CMake](https://cmake.org/download/) (Add to PATH during install)
- [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/) with "Desktop development with C++" workload and the "C++ Clang tools for Windows" component (clang-cl/llvm-cov/llvm-profdata)
- [Ninja](https://ninja-build.org/) — lightweight build system
- [Node.js](https://nodejs.org/) with npm

### Steps

1. **Download and install runtime dependencies**

   - Download the Ultralight SDK from https://ultralig.ht/download and extract it somewhere on your machine.

     CMake will try to autolocate the SDK at the following paths
   
     - `%PROGRAMFILES%/Ultralight/SDK`
     - `%HOME/Ultralight/SDK`
   
     But you can manually provide the path with the CMake flag `-DULTRALIGHT_SDK_PATH`

2. **Build the UI**

   From inside the `ui` folder package the React application using npm.

   ```powershell
   npm install
   npm run build
   ```

   Copy the content of `ui/dist` to `runtime/assets`.

3. **Build the runtime with CMake**

   Make sure the Visual Studio Developer PowerShell (x64) is active so the VS toolchain and tools (for example `clang-cl`, `llvm-cov`, `llvm-profdata`, `link`, and `ninja`) are available.

   ```powershell
   Import-Module 'C:\\Program Files\\Microsoft Visual Studio\\...\\Common7\\Tools\\Microsoft.VisualStudio.DevShell.dll' 
   Enter-VsDevShell -VsInstallPath: 'C:\\Program Files\\Microsoft Visual Studio\\...' -DevCmdArguments '-arch=x64'
   ```

   After that you can build the runtime by using CMake and Ninja from inside the `runtime` folder. CMakeLists defaults Ninja builds to clang-cl, and selects the ClangCL toolset for the VS generator.

   ```powershell
   cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```

   To build and launch the runtime in dev mode (points at the UI dev server on localhost:5173 by default) in one step, use the `run-dev` target:

   ```powershell
   cmake --build build --config Release --target run-dev
   ```

   You can change the dev server port by re-configuring with `-DFBXEX_DEV_PORT=3000`.

4. **Run the application**
   
   `.runtime/build/Release/fbxex.exe`

## Running unit tests

### Runtime tests (C++/Catch2)

From inside `runtime` (after configuring the SDK paths as in the build section):

```powershell
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Debug -DFBXEX_ENABLE_COVERAGE=ON
cmake --build build --target fbxex_tests
ctest --test-dir build --output-on-failure
```

Coverage (clang-cl + llvm-cov) after running tests:

```powershell
ctest --test-dir build --output-on-failure
llvm-profdata merge -sparse "$PWD/build/tests/fbxex_tests_*.profraw" -o "$PWD/build/coverage.profdata"
llvm-cov report "$PWD/build/tests/fbxex_tests.exe" `
  -instr-profile="$PWD/build/coverage.profdata" `
  -path-equivalence="$PWD","$PWD" `
  -ignore-filename-regex="ultralight_adapters.cpp" `
  src
```

### UI tests (React)

From inside `ui`:

```powershell
npm install
npm run test
```

Coverage report can be obtained via

```
npm run test -- --coverage --watch=false
``` 
