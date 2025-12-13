# fbxex - The FBX Explorer & Inspector

A lightweight Windows desktop application for exploring and inspecting FBX files.

## Contributing

### Prerequisites

- [Git](https://gitforwindows.org/)
- [CMake](https://cmake.org/download/) (Add to PATH during install)
- [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/) with "Desktop development with C++" workload
- [Ninja](https://ninja-build.org/) — lightweight build system

### Setup

1. **Download and install dependencies**

   - Download the Ultralight SDK from https://ultralig.ht/download and extract it somewhere on your machine.

     CMake will try to autolocate the SDK at the following paths
   
     - `%PROGRAMFILES%/Ultralight/SDK`
     - `%HOME/Ultralight/SDK`
   
     But you can manually provide the path with the CMake flag `-DULTRALIGHT_SDK_PATH`

2. **Build with CMake**

   Make sure the Visual Studio Developer PowerShell (x64) is active so the MSVC toolchain and tools (for example `cl`, `link`, and `ninja`) are available.

   ```powershell
   Import-Module 'C:\\Program Files\\Microsoft Visual Studio\\...\\Common7\\Tools\\Microsoft.VisualStudio.DevShell.dll' 
   Enter-VsDevShell -VsInstallPath: 'C:\\Program Files\\Microsoft Visual Studio\\...' -DevCmdArguments '-arch=x64'
   ```

    After that you can build by using CMake and Ninja

   ```powershell
   cmake -B build -G Ninja
   cmake --build build --config Release
   ```

3. **Run the application**
   
   `./build/Release/fbxex.exe`

