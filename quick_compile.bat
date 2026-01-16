@echo off
echo Compiling CyberForce (simple version)...
echo.
C:\msys64\ucrt64.exe -here -c "gcc -o cyberforce.exe simple_cyberforce.c -O2 -Wall 2>&1; if [ -f cyberforce.exe ]; then echo ''; echo 'SUCCESS! Executable created.'; ls -lh cyberforce.exe; echo ''; ./cyberforce.exe --version; else echo ''; echo 'FAILED! Checking errors above...'; fi; echo ''; echo 'Press Enter to continue...'; read"
