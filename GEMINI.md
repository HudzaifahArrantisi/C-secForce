# CyberForce Project Overview

## Project Type
Code Project (C-based Cybersecurity Testing Framework)

## Project Overview
CyberForce adalah framework pengujian penetrasi (penetration testing) keamanan siber yang ditulis menggunakan bahasa C. Tools ini dirancang untuk mendukung berbagai serangan brute force pada beberapa protokol jaringan seperti HTTP/HTTPS, FTP, SSH, dan MySQL. 

**Fitur Utama:**
- Mendukung berbagai protokol jaringan (HTTP/HTTPS, FTP, SSH, MySQL).
- Fitur keamanan lanjutan seperti *Rate Limiting*, *Proxy Rotation*, *Multi-Threading* (hingga 100 threads), dan *Custom Wordlist*.
- Termasuk rotasi *User-Agent* dan penyesuaian pola respons (*Pattern Matching*).
- Sistem Logging & Reporting yang detail dengan kemampuan export (JSON/CSV).
- *Cross-platform* mendukung kompilasi di Linux dan Windows.

## Arsitektur dan Struktur Direktori
- `src/`: Berisi seluruh kode sumber utama proyek.
  - `main.c`: Titik masuk aplikasi.
  - `core/`: Berisi logika inti seperti mesin serang (*attack engine*), manajemen *thread*, pembatasan *rate limit*, dan *handler* protokol.
  - `modules/`: Modul implementasi untuk masing-masing protokol (http, ftp, ssh, mysql).
  - `utils/`: Utilitas bantu seperti pencatatan (*logger*), pembuat daftar kata (*wordlist generator*), pemutar proxy (*proxy rotator*), utilitas kriptografi, dan *pattern matcher*.
  - `include/`: File *header*.
- `data/`: Memuat file yang digunakan dalam pengujian, seperti daftar agen pengguna dan *wordlist* (`common_users.txt`, `common_passwords.txt`, dll).
- `examples/`: Skrip uji dan konfigurasi contoh.
- `Makefile`, `build.bat`, `quick_compile.bat`: Skrip *build* untuk berbagai platform (Linux/Windows).

## Building and Running

Proyek ini dapat dikompilasi menggunakan dependensi sistem atau skrip *build* sederhana. Terdapat dua pendekatan kompilasi di Windows:

**1. Menggunakan Skrip *Build* Sederhana (Standalone):**
Paling cocok untuk kompilasi secara instan tanpa pustaka eksternal yang kompleks.
```bash
# Menjalankan build cepat di Windows (dengan dukungan MinGW)
.\quick_compile.bat
```

**2. Menggunakan Versi Lengkap (Membutuhkan MSYS2 dan Dependensi):**
```bash
# Instal dependensi (Di MSYS2 Terminal):
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-libssh mingw-w64-ucrt-x86_64-openssl

# Menjalankan skrip build di Command Prompt/PowerShell
.\build.bat

# Atau Menggunakan Makefile (Linux/MSYS2 Terminal):
mingw32-make clean
mingw32-make all
```

**Menjalankan Aplikasi:**
```bash
# Melihat daftar bantuan dan panduan penggunaan
.\cyberforce.exe --help

# Contoh menjalankan brute force HTTP
.\cyberforce.exe -t http://example.com/admin -u data/wordlists/common_users.txt -p data/wordlists/common_passwords.txt -T 10
```

## Development Conventions
- **Bahasa:** C dengan kompatibilitas POSIX (*Linux*) dan modifikasi *Windows* (*cross-platform* dengan *conditional compilation* `#ifdef _WIN32`).
- **Dependencies:** Proyek ini bergantung pada `libcurl`, `libssh`, `openssl`, dan implementasi soket platform-spesifik.
- **Etika Penggunaan:** Ini adalah alat pendidikan dan hanya boleh digunakan untuk pengujian keamanan yang sah (*authorized security testing*).
