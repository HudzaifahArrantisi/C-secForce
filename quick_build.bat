@echo off
echo ========================================
echo   CyberForce - Quick Build Test
echo ========================================
echo.

echo [*] Testing GCC...
gcc --version
if errorlevel 1 (
    echo [!] GCC not found!
    pause
    exit /b 1
)

echo.
echo [*] Compiling simple test...
gcc -o cyberforce.exe simple_cyberforce.c -O2 -Wall
if errorlevel 1 (
    echo [!] Compilation failed!
    echo.
    echo Trying again with verbose output...
    gcc -o cyberforce.exe simple_cyberforce.c -O2 -Wall -v
    pause
    exit /b 1
)

echo.
if exist cyberforce.exe (
    echo [+] SUCCESS! Executable created: cyberforce.exe
    echo.
    dir cyberforce.exe
    echo.
    echo [*] Testing execution...
    echo.
    cyberforce.exe --version
    echo.
    echo [+] Program works correctly!
    echo.
    echo Try running: cyberforce.exe --help
) else (
    echo [!] ERROR: cyberforce.exe not found after compilation
    echo This might be due to:
    echo   - Antivirus blocking the file
    echo   - Permission issues
    echo   - Path problems
)

echo.
pause
