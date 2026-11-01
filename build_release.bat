@echo off
REM ============================================================================
REM Unified Engineering Workbench - Windows 11 x64 Production Release Build
REM Statically links the MSVC C++ runtime (/MT) to ensure 0 external DLL dependencies.
REM ============================================================================

setlocal enabledelayedexpansion

echo [1/5] Checking Build Environment...
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake is not found in PATH. Please install CMake 3.16+ or open Developer Command Prompt for VS.
    exit /b 1
)

echo [2/5] Configuring Release Build with Static Runtime (/MT)...
cmake -B build_win -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

echo [3/5] Compiling Binaries (Launch.exe and workbench_server.exe)...
cmake --build build_win --config Release --parallel
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build compilation failed.
    exit /b 1
)

echo [4/5] Running Automated Unit Tests via CTest...
ctest --test-dir build_win -C Release --output-on-failure
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Automated test suite failed! Packaging aborted.
    exit /b 1
)

echo [5/5] Assembling Standalone Distribution Package in dist/...
if exist dist rmdir /s /q dist
mkdir dist
mkdir dist\web
mkdir dist\data

REM Copy binaries
copy build_win\Release\Launch.exe dist\Launch.exe >nul
copy build_win\Release\workbench_server.exe dist\workbench_server.exe >nul

REM Copy assets
xcopy /E /I /Y web dist\web >nul
xcopy /E /I /Y data dist\data >nul

echo ============================================================================
echo [SUCCESS] Windows 11 Distribution Package Ready in dist/
echo Directory Layout:
echo   dist\Launch.exe            (Double-clickable launcher, silent, opens browser)
echo   dist\workbench_server.exe  (Statically-linked C++ backend HTTP server)
echo   dist\web\                  (Offline HTML/CSS/JS/Three.js assets)
echo   dist\data\                 (Synthetic STL and ANSYS CSV models)
echo ============================================================================
pause
