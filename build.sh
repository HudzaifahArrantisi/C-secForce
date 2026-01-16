#!/bin/bash
# CyberForce Build Script - Bash version untuk MSYS2/Git Bash

echo "========================================"
echo "  CyberForce Windows Build"
echo "========================================"
echo ""

# Check if running in proper environment
if ! command -v gcc &> /dev/null; then
    echo "[!] GCC not found!"
    echo "Please run this in MSYS2 MinGW 64-bit terminal"
    exit 1
fi

echo "[*] Compiler: $(gcc --version | head -n1)"
echo ""

# Source files
SOURCES=(
    "src/main.c"
    "src/core/attack_engine.c"
    "src/core/thread_manager.c"
    "src/core/rate_limiter.c"
    "src/core/protocol_handlers.c"
    "src/modules/http_brute.c"
    "src/modules/ftp_brute.c"
    "src/modules/ssh_brute.c"
    "src/modules/mysql_brute.c"
    "src/utils/wordlist_gen.c"
    "src/utils/pattern_matcher.c"
    "src/utils/proxy_rotator.c"
    "src/utils/logger.c"
    "src/utils/crypto_helper.c"
)

OBJECTS=()
FAILED=0

for src in "${SOURCES[@]}"; do
    obj="${src%.c}.o"
    echo "[*] Compiling $src..."
    
    if gcc -Wall -O2 -D_WIN32 -Isrc/include -c "$src" -o "$obj" 2>&1; then
        OBJECTS+=("$obj")
        echo "    ✓ OK"
    else
        echo "    ✗ FAILED"
        FAILED=1
        break
    fi
done

if [ $FAILED -eq 1 ]; then
    echo ""
    echo "[!] Compilation failed!"
    exit 1
fi

echo ""
echo "[*] Linking..."

if gcc -o cyberforce.exe "${OBJECTS[@]}" -lcurl -lssh -lssl -lcrypto -lws2_32 -lm -lpthread 2>&1; then
    echo ""
    echo "[+] Build successful!"
    echo "[+] Executable: cyberforce.exe"
    echo ""
    ls -lh cyberforce.exe
    echo ""
    echo "Try: ./cyberforce.exe --help"
else
    echo ""
    echo "[!] Linking failed - missing libraries"
    echo ""
    echo "Install dependencies:"
    echo "  pacman -S mingw-w64-x86_64-curl"
    echo "  pacman -S mingw-w64-x86_64-libssh"
    echo "  pacman -S mingw-w64-x86_64-openssl"
fi

echo ""
