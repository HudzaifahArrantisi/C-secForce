# CyberForce - Quick Build Guide for Windows

## Saya sudah memperbaiki kode agar kompatibel dengan Windows!

### Perubahan yang Dilakukan:

1. **FTP Module** ([src/modules/ftp_brute.c](src/modules/ftp_brute.c))
   - Menambahkan dukungan Winsock2 untuk Windows
   - Inisialisasi WSAStartup otomatis
   - Menggunakan closesocket() untuk Windows

2. **Rate Limiter** ([src/core/rate_limiter.c](src/core/rate_limiter.c))
   - Implementasi clock_gettime() dan nanosleep() untuk Windows
   - Menggunakan Windows API (GetSystemTimeAsFileTime, Sleep)

3. **Makefile** ([Makefile](Makefile))
   - Deteksi otomatis OS (Windows/Linux)
   - Link dengan ws2_32.lib untuk Winsock
   - Conditional compilation flags

4. **File Baru:**
   - `src/modules/mysql_brute.c` - Placeholder untuk MySQL module
   - `src/core/protocol_handlers.c` - Handler untuk semua protokol
   - `BUILD_WINDOWS.md` - Panduan instalasi lengkap

## Cara Build (Tanpa MSYS2)

### Menggunakan GCC di PowerShell

```powershell
# Compile semua file source
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/main.c -o src/main.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/core/attack_engine.c -o src/core/attack_engine.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/core/thread_manager.c -o src/core/thread_manager.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/core/rate_limiter.c -o src/core/rate_limiter.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/core/protocol_handlers.c -o src/core/protocol_handlers.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/modules/http_brute.c -o src/modules/http_brute.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/modules/ftp_brute.c -o src/modules/ftp_brute.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/modules/ssh_brute.c -o src/modules/ssh_brute.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/modules/mysql_brute.c -o src/modules/mysql_brute.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/wordlist_gen.c -o src/utils/wordlist_gen.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/pattern_matcher.c -o src/utils/pattern_matcher.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/proxy_rotator.c -o src/utils/proxy_rotator.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/logger.c -o src/utils/logger.o
gcc -Wall -Wextra -O2 -D_WIN32 -Isrc/include -c src/utils/crypto_helper.c -o src/utils/crypto_helper.o

# Link semua object files
gcc -o cyberforce.exe src/main.o src/core/attack_engine.o src/core/thread_manager.o src/core/rate_limiter.o src/core/protocol_handlers.o src/modules/http_brute.o src/modules/ftp_brute.o src/modules/ssh_brute.o src/modules/mysql_brute.o src/utils/wordlist_gen.o src/utils/pattern_matcher.o src/utils/proxy_rotator.o src/utils/logger.o src/utils/crypto_helper.o -lcurl -lssh -lssl -lcrypto -lws2_32 -lm -lpthread
```

## Cara Run

```powershell
# Tampilkan bantuan
.\cyberforce.exe --help

# Contoh HTTP brute force
.\cyberforce.exe -t http://example.com/login -m http -U users.txt -P passwords.txt

# Contoh FTP brute force
.\cyberforce.exe -t ftp.example.com -m ftp -U users.txt -P passwords.txt -p 21

# Contoh SSH brute force
.\cyberforce.exe -t 192.168.1.100 -m ssh -U users.txt -P passwords.txt -p 22
```

## Troubleshooting

### Error: undefined reference to functions

Pastikan semua libraries terinstall:
- libcurl
- libssh
- openssl
- pthread (biasanya sudah include di MinGW)

### Error: cannot find -lcurl

Install libraries menggunakan package manager atau download manual:

**Menggunakan MSYS2:**
```bash
pacman -S mingw-w64-x86_64-curl
pacman -S mingw-w64-x86_64-libssh
pacman -S mingw-w64-x86_64-openssl
```

### DLL not found saat run

Copy DLL yang diperlukan ke folder executable atau tambahkan ke PATH:
```powershell
$env:PATH = "C:\path\to\dlls;$env:PATH"
```

## Catatan Penting

- Tool ini untuk tujuan edukasi saja
- Memerlukan otorisasi sebelum melakukan testing
- Lihat SECURITY.md untuk panduan responsible disclosure
- Lihat USAGE.md untuk contoh penggunaan lengkap

## Struktur File yang Diperbaiki

```
c-secforce/
├── src/
│   ├── main.c (sudah ada)
│   ├── core/
│   │   ├── attack_engine.c (sudah ada)
│   │   ├── thread_manager.c (sudah ada)
│   │   ├── rate_limiter.c (✓ DIPERBAIKI - Windows support)
│   │   └── protocol_handlers.c (✓ BARU)
│   ├── modules/
│   │   ├── http_brute.c (sudah ada)
│   │   ├── ftp_brute.c (✓ DIPERBAIKI - Winsock support)
│   │   ├── ssh_brute.c (sudah ada)
│   │   └── mysql_brute.c (✓ BARU)
│   └── utils/
│       └── (semua file sudah ada)
├── Makefile (✓ DIPERBAIKI - Windows detection)
├── BUILD_WINDOWS.md (✓ BARU - Panduan ini)
└── build.ps1 (✓ BARU - PowerShell build script)
```

## Kesimpulan

Semua file sudah diperbaiki untuk kompatibilitas Windows. Anda bisa compile menggunakan GCC yang sudah terinstall di sistem Anda. Jika ada error terkait missing libraries, install melalui MSYS2 atau download pre-compiled binaries.

Untuk pertanyaan lebih lanjut, lihat dokumentasi lengkap di BUILD_WINDOWS.md.
