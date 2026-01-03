@echo off
cd build-msvc
cmake .. -G "Visual Studio 18 2026" -A x64
cmake --build . --config Release
echo.
echo Build complete! Executable location:
echo %CD%\Release\bench.exe
