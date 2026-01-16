# CyberForce - Build Instructions for Windows

## Prerequisites

### 1. Install MinGW-w64 (if not already installed)

Download and install from: https://www.mingw-w64.org/
Or use winget:
```powershell
winget install -e --id=LLVM.LLVM
```

### 2. Install Required Libraries

#### Option A: Using MSYS2 (Recommended)

1. Download MSYS2 from: https://www.msys2.org/
2. Install and update MSYS2:
```bash
pacman -Syu
```

3. Install development tools and libraries:
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-curl
pacman -S mingw-w64-x86_64-libssh
pacman -S mingw-w64-x86_64-openssl
pacman -S make
```

4. Add MSYS2 to your PATH:
```powershell
$env:PATH = "C:\msys2\mingw64\bin;$env:PATH"
```

#### Option B: Manual Installation

Download pre-compiled libraries:
- libcurl: https://curl.se/download.html
- libssh: https://www.libssh.org/
- OpenSSL: https://slproweb.com/products/Win32OpenSSL.html

## Building

### Using PowerShell Build Script (Easiest)

```powershell
# Clean previous build
.\build.ps1 clean

# Build the project
.\build.ps1 build

# Run with help
.\build.ps1 run
```

### Using Make (if MSYS2 installed)

```bash
make clean
make all
make run
```

### Manual Compilation

```powershell
# Set include and library paths
$env:CPATH = "C:\path\to\include"
$env:LIBRARY_PATH = "C:\path\to\lib"

# Compile
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/*.c src/**/*.c
gcc -o cyberforce.exe *.o -lcurl -lssh -lssl -lcrypto -lws2_32 -lm -lpthread
```

## Running

### Basic Usage

```powershell
# Show help
.\cyberforce.exe --help

# HTTP brute force example
.\cyberforce.exe -t http://example.com/login -m http -U users.txt -P passwords.txt

# FTP brute force example
.\cyberforce.exe -t ftp.example.com -m ftp -U users.txt -P passwords.txt -p 21

# SSH brute force example
.\cyberforce.exe -t 192.168.1.100 -m ssh -U users.txt -P passwords.txt -p 22
```

## Troubleshooting

### Error: Cannot find -lcurl

Install libcurl or add library path:
```powershell
$env:LIBRARY_PATH = "C:\msys64\mingw64\lib;$env:LIBRARY_PATH"
```

### Error: Cannot find headers

Add include path:
```powershell
$env:CPATH = "C:\msys64\mingw64\include;$env:CPATH"
```

### DLL not found when running

Copy DLLs to executable directory or add to PATH:
```powershell
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
```

## Quick Start with MSYS2 MinGW64

The fastest way to build and run:

```powershell
# 1. Open MSYS2 MinGW 64-bit terminal

# 2. Navigate to project directory
cd /c/Users/DELL/OneDrive/ドキュメント/ALL\ in\ ONE/TOOLS/c-secforce

# 3. Build
make clean
make all

# 4. Run
./cyberforce.exe --help
```

## Notes

- This tool is for educational purposes only
- Requires proper authorization before testing
- See SECURITY.md for responsible disclosure guidelines
- See USAGE.md for detailed usage examples
