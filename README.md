# fbxex - The FBX Explorer & Inspector

A lightweight Windows desktop application for exploring and inspecting FBX files.

## Contributing - Building from Source

### Prerequisites

- [Git](https://gitforwindows.org/)
- [CMake](https://cmake.org/download/) (Add to PATH during install)
- [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/) with "Desktop development with C++" workload
- [Ninja](https://ninja-build.org/) — lightweight build system
- [Node.js](https://nodejs.org/) with npm

### Steps

1. **Download and install runtime dependencies**

   - Download the Ultralight SDK from https://ultralig.ht/download and extract it somewhere on your machine.

     CMake will try to autolocate the SDK at the following paths
   
     - `%PROGRAMFILES%/Ultralight/SDK`
     - `%HOME/Ultralight/SDK`
   
     But you can manually provide the path with the CMake flag `-DULTRALIGHT_SDK_PATH`

   - Download the FBX SDK from https://aps.autodesk.com/developer/overview/fbx-sdk and install it somewhere on your machine.

     CMake will try to autolocate the SDK at the following paths (`*` indicates check on all subfolders)

     - `%PROGRAMFILES%/Autodesk/FBX/FBX SDK/*`
     - `%HOME/Autodesk/FBX/FBX SDK/*`

     But you can manually provide the path with the CMake flag `-DFBX_SDK_PATH`

2. **Build the UI**

   From inside the `ui` folder package the React application using npm.

   ```powershell
   npm install
   npm run build
   ```

   Copy the content of `ui/dist` to `runtime/assets`.

3. **Build the runtime with CMake**

   Make sure the Visual Studio Developer PowerShell (x64) is active so the MSVC toolchain and tools (for example `cl`, `link`, and `ninja`) are available.

   ```powershell
   Import-Module 'C:\\Program Files\\Microsoft Visual Studio\\...\\Common7\\Tools\\Microsoft.VisualStudio.DevShell.dll' 
   Enter-VsDevShell -VsInstallPath: 'C:\\Program Files\\Microsoft Visual Studio\\...' -DevCmdArguments '-arch=x64'
   ```

   After that you can build the runtime by using CMake and Ninja from inside the `runtime` folder.

   ```powershell
   cmake -B build -G Ninja
   cmake --build build --config Release
   ```

4. **Run the application**
   
   `.runtime/build/Release/fbxex.exe`

