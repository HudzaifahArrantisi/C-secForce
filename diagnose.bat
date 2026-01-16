@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   DIAGNOSIS ERROR COMPILATION
echo ========================================
echo.

echo [*] GCC Version:
gcc --version | findstr gcc
echo.

echo ========================================
echo   TEST 1: ANTIVIRUS CHECK
echo ========================================
echo.
echo [*] Checking Windows Defender status...
powershell -Command "Get-MpComputerStatus | Select-Object RealTimeProtectionEnabled, IoavProtectionEnabled"
echo.

echo ========================================
echo   TEST 2: COMPILE TEST (C:\)
echo ========================================
echo.
cd C:\
if not exist test_diagnosis mkdir test_diagnosis
cd test_diagnosis

echo [*] Creating test.c...
echo #include ^<stdio.h^> > test.c
echo int main(){ printf("OK\n"); return 0; } >> test.c

echo [*] Compiling...
gcc test.c -o test.exe 2>&1

echo [*] Checking result...
if exist test.exe (
    echo [+] SUCCESS - test.exe created!
    echo [+] File size:
    dir test.exe | findstr test.exe
    echo [+] Running test...
    test.exe
    echo.
    echo ^>^> KESIMPULAN: GCC WORKING! Masalah mungkin di path project.
) else (
    echo [-] FAILED - test.exe NOT created
    echo.
    echo ^>^> KESIMPULAN: Antivirus blocking atau GCC corrupt!
)
echo.

echo ========================================
echo   TEST 3: PERMISSION CHECK  
echo ========================================
echo.
echo test > test_write.txt
if exist test_write.txt (
    echo [+] Write permission OK
    del test_write.txt
) else (
    echo [-] Write permission DENIED
)
echo.

echo ========================================
echo   TEST 4: PATH CHECK
echo ========================================
echo.
echo [*] Current GCC path:
where gcc
echo.
echo [*] Linker available:
where ld 2>nul
if errorlevel 1 (
    echo [-] Linker (ld) NOT FOUND - GCC incomplete!
) else (
    echo [+] Linker found
)
echo.

echo ========================================
echo   RECOMMENDATION
echo ========================================
echo.

if exist C:\test_diagnosis\test.exe (
    echo Status: GCC is WORKING
    echo.
    echo Possible causes of your error:
    echo   1. Unicode path issue (folder with Japanese chars)
    echo   2. Antivirus blocking in that specific folder
    echo.
    echo Solutions:
    echo   - Copy project to C:\cyberforce
    echo   - Add Windows Defender exclusion
    echo   - Use MSYS2 for compilation
) else (
    echo Status: GCC is NOT WORKING
    echo.
    echo Possible causes:
    echo   1. Windows Defender blocking (90 percent)
    echo   2. Antivirus software blocking
    echo   3. GCC installation corrupt
    echo.
    echo Solutions:
    echo   A. Disable Windows Defender temporarily:
    echo      PowerShell (Run as Admin):
    echo      Set-MpPreference -DisableRealtimeMonitoring $true
    echo.
    echo   B. Add folder exclusion in Windows Security
    echo.
    echo   C. Install MSYS2 (RECOMMENDED):
    echo      Download: https://www.msys2.org/
)

echo.
echo ========================================
cd C:\test_diagnosis
echo Output saved in: C:\test_diagnosis\
echo.
pause
