@echo off
echo Starting MSYS2 UCRT64 terminal to build CyberForce...
echo.
C:\msys64\ucrt64.exe -where "%~dp0" -c "pwd && mingw32-make clean && mingw32-make all && echo. && echo Build complete! && echo. && ./cyberforce.exe --version && echo. && echo Press any key to exit... && read"
