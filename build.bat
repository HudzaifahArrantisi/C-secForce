@echo off
echo Building CyberForce...
echo.

echo [*] Compiling main.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/main.c -o src/main.o
if errorlevel 1 goto error

echo [*] Compiling attack_engine.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/core/attack_engine.c -o src/core/attack_engine.o
if errorlevel 1 goto error

echo [*] Compiling thread_manager.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/core/thread_manager.c -o src/core/thread_manager.o
if errorlevel 1 goto error

echo [*] Compiling rate_limiter.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/core/rate_limiter.c -o src/core/rate_limiter.o
if errorlevel 1 goto error

echo [*] Compiling protocol_handlers.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/core/protocol_handlers.c -o src/core/protocol_handlers.o
if errorlevel 1 goto error

echo [*] Compiling http_brute.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/modules/http_brute.c -o src/modules/http_brute.o
if errorlevel 1 goto error

echo [*] Compiling ftp_brute.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/modules/ftp_brute.c -o src/modules/ftp_brute.o
if errorlevel 1 goto error

echo [*] Compiling ssh_brute.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/modules/ssh_brute.c -o src/modules/ssh_brute.o
if errorlevel 1 goto error

echo [*] Compiling mysql_brute.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/modules/mysql_brute.c -o src/modules/mysql_brute.o
if errorlevel 1 goto error

echo [*] Compiling wordlist_gen.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/wordlist_gen.c -o src/utils/wordlist_gen.o
if errorlevel 1 goto error

echo [*] Compiling pattern_matcher.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/pattern_matcher.c -o src/utils/pattern_matcher.o
if errorlevel 1 goto error

echo [*] Compiling proxy_rotator.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/proxy_rotator.c -o src/utils/proxy_rotator.o
if errorlevel 1 goto error

echo [*] Compiling logger.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/logger.c -o src/utils/logger.o
if errorlevel 1 goto error

echo [*] Compiling crypto_helper.c...
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/crypto_helper.c -o src/utils/crypto_helper.o
if errorlevel 1 goto error

echo.
echo [*] Linking...
gcc -o cyberforce.exe src/main.o src/core/attack_engine.o src/core/thread_manager.o src/core/rate_limiter.o src/core/protocol_handlers.o src/modules/http_brute.o src/modules/ftp_brute.o src/modules/ssh_brute.o src/modules/mysql_brute.o src/utils/wordlist_gen.o src/utils/pattern_matcher.o src/utils/proxy_rotator.o src/utils/logger.o src/utils/crypto_helper.o -lcurl -lssh -lssl -lcrypto -lws2_32 -lm -lpthread
if errorlevel 1 goto linkerror

echo.
echo [+] Build successful!
echo [+] Executable: cyberforce.exe
goto end

:linkerror
echo.
echo [!] Linking failed - missing libraries
echo.
echo Note: The following libraries are required:
echo   - libcurl
echo   - libssh
echo   - openssl (libssl, libcrypto)
echo.
echo Install via MSYS2:
echo   pacman -S mingw-w64-x86_64-curl
echo   pacman -S mingw-w64-x86_64-libssh
echo   pacman -S mingw-w64-x86_64-openssl
echo.
goto end

:error
echo.
echo [!] Compilation failed!
goto end

:end
pause
