# CyberForce v2.0.0

Framework keamanan siber berbasis C untuk pengujian penetrasi multi-protokol dengan dukungan brute force pada HTTP, FTP, SSH, dan MySQL.

## 📋 Daftar Isi

- [Deskripsi Proyek](#deskripsi-proyek)
- [Fitur Utama](#fitur-utama)
- [Struktur Proyek](#struktur-proyek)
- [Alur Kerja Sistem](#alur-kerja-sistem)
- [Permasalahan & Solusi](#permasalahan--solusi)
- [Instalasi](#instalasi)
- [Cara Penggunaan](#cara-penggunaan)
- [Kompilasi](#kompilasi)
- [Dependensi](#dependensi)

---

## 🎯 Deskripsi Proyek

CyberForce adalah tools security testing yang dirancang untuk melakukan pengujian keamanan pada berbagai protokol jaringan. Tools ini menggunakan teknik brute force dengan fitur rate limiting, proxy rotation, dan multi-threading untuk efisiensi maksimal.

### Tujuan Proyek
- Menyediakan framework testing keamanan yang fleksibel
- Mendukung multiple protokol (HTTP, FTP, SSH, MySQL)
- Optimasi performa dengan multi-threading
- Kompatibilitas cross-platform (Linux & Windows)

---

## ✨ Fitur Utama

### 1. **Multi-Protocol Support**
   - HTTP/HTTPS Basic Auth & Form-based Auth
   - FTP Authentication
   - SSH Key & Password Auth
   - MySQL Database Auth

### 2. **Advanced Features**
   - **Rate Limiting**: Kontrol kecepatan request untuk menghindari deteksi
   - **Proxy Rotation**: Rotasi proxy otomatis untuk anonymitas
   - **Multi-Threading**: Parallel execution hingga 100 threads
   - **Custom Wordlist**: Support untuk wordlist kustom
   - **User-Agent Rotation**: Random user-agent untuk web requests
   - **Pattern Matching**: Custom pattern untuk response analysis

---

## 🚀 Tutorial Step-by-Step (Quick Start)

Ikuti langkah-langkah berikut untuk menjalankan CyberForce di sistem Anda:

### Langkah 1: Persiapan Lingkungan (Environment Setup)

**Untuk Windows (Menggunakan MSYS2):**
1. Download dan Install [MSYS2](https://www.msys2.org/).
2. Buka terminal **MSYS2 UCRT64**.
3. Jalankan perintah berikut untuk menginstall compiler dan library:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-libssh mingw-w64-ucrt-x86_64-openssl
   ```

**Untuk Linux (Debian/Ubuntu):**
```bash
sudo apt-get update
sudo apt-get install build-essential libcurl4-openssl-dev libssh-dev libssl-dev
```

### Langkah 2: Kompilasi Proyek (Building)

Pilih salah satu metode kompilasi berikut:

*   **Metode Cepat (Windows - Tanpa Library Eksternal):**
    ```powershell
    .\quick_compile.bat
    ```
*   **Metode Lengkap (Menggunakan Make):**
    ```bash
    # Di MSYS2 atau Linux terminal
    mingw32-make all   # Untuk Windows
    make all           # Untuk Linux
    ```

### Langkah 3: Menyiapkan Wordlist

Pastikan Anda memiliki file username dan password di folder `data/wordlists/`. Anda bisa menggunakan file bawaan:
- `data/wordlists/common_users.txt`
- `data/wordlists/common_passwords.txt`

### Langkah 4: Menjalankan Serangan (Running)

Gunakan perintah berikut untuk memulai pengujian (contoh HTTP):
```bash
.\cyberforce.exe -t http://example.com/login -u data/wordlists/common_users.txt -p data/wordlists/common_passwords.txt -T 10
```

### Langkah 5: Melihat Hasil

CyberForce akan menampilkan hasil secara real-time di terminal. Jika serangan berhasil, detail credential akan muncul dengan tanda `[SUCCESS]`.

---

## 📁 Struktur Proyek

```
c-secforce/
├── src/
│   ├── main.c                      # Entry point aplikasi
│   ├── include/
│   │   ├── cyberforce.h           # Header utama
│   │   ├── defines.h              # Konstanta & defines
│   │   └── protocols.h            # Protocol definitions
│   ├── core/
│   │   ├── attack_engine.c        # Engine utama untuk attack orchestration
│   │   ├── protocol_handlers.c    # Routing protokol ke modul terkait
│   │   ├── rate_limiter.c         # Rate limiting implementation
│   │   └── thread_manager.c       # Thread pool & management
│   ├── modules/
│   │   ├── http_brute.c           # HTTP/HTTPS brute force module
│   │   ├── ftp_brute.c            # FTP brute force module
│   │   ├── ssh_brute.c            # SSH brute force module
│   │   └── mysql_brute.c          # MySQL brute force module
│   └── utils/
│       ├── logger.c               # Logging system
│       ├── wordlist_gen.c         # Wordlist generator
│       ├── pattern_matcher.c      # Pattern matching untuk response
│       ├── proxy_rotator.c        # Proxy rotation logic
│       └── crypto_helper.c        # Cryptographic utilities
├── data/
│   ├── wordlists/
│   │   ├── common_passwords.txt   # Password wordlist
│   │   ├── common_users.txt       # Username wordlist
│   │   └── user_agents.txt        # User-Agent list
│   └── user_agents.txt
├── examples/
│   ├── config.json                # Example configuration
│   └── scripts/
│       └── test_http.sh           # Test script untuk HTTP module
├── Makefile                       # Build automation
├── Dockerfile                     # Container deployment
├── simple_cyberforce.c            # Standalone version (tanpa dependencies)
├── build_msys2.bat                # Build script untuk MSYS2
├── quick_compile.bat              # Quick compilation script
└── README.md                      # Dokumentasi ini
```

---

## 🔄 Alur Kerja Sistem

### 1. **Inisialisasi (main.c)**
```
User Input → Parse Arguments → Validate Parameters → Initialize Components
```
- User memberikan target, protokol, wordlist
- Parsing menggunakan getopt (atau stub untuk Windows)
- Validasi parameter (URL format, file exists, thread count)
- Setup logger, rate limiter, thread pool

### 2. **Attack Engine (attack_engine.c)**
```
Load Wordlist → Dispatch to Protocol Handler → Execute Attack → Collect Results
```
- Membaca wordlist dari file atau generate otomatis
- Menentukan protokol handler yang sesuai
- Membagi workload ke multiple threads
- Mengumpulkan hasil dan statistik

### 3. **Protocol Handler (protocol_handlers.c)**
```
Identify Protocol → Route to Module → Execute Module Function
```
- HTTP → `http_attack()` di http_brute.c
- FTP → `ftp_attack()` di ftp_brute.c
- SSH → `ssh_attack()` di ssh_brute.c
- MySQL → `mysql_attack()` di mysql_brute.c

### 4. **Module Execution (contoh: http_brute.c)**
```
Initialize Connection → Apply Rate Limit → Send Request → Analyze Response → Log Result
```
- Setup socket/curl connection
- Apply delay sesuai rate limit
- Kirim authentication request
- Check response code (200, 401, 403, etc.)
- Log success/failure dengan detail timing

### 5. **Thread Management (thread_manager.c)**
```
Create Thread Pool → Assign Tasks → Monitor Progress → Join Threads → Aggregate Results
```
- Membuat pool dengan N threads
- Mendistribusikan credential pairs ke threads
- Monitor status setiap thread
- Cleanup setelah selesai

### 6. **Rate Limiting (rate_limiter.c)**
```
Check Current Rate → Calculate Delay → Sleep if Needed → Update Counter
```
- Mencegah flood request ke target
- Implementasi token bucket algorithm
- Dynamic adjustment berdasarkan response time

---

## ⚠️ Permasalahan & Solusi

### **Problem 1: Kompilasi Error pada Windows**

#### **Masalah:**
```
fatal error: unistd.h: No such file or directory
fatal error: arpa/inet.h: No such file or directory
fatal error: sys/socket.h: No such file or directory
```

#### **Penyebab:**
- Header POSIX (unistd.h, arpa/inet.h, sys/socket.h) tidak tersedia di Windows
- Kode awal ditulis untuk Linux/Unix system

#### **Solusi:**
Menambahkan conditional compilation untuk Windows:
```c
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netdb.h>
#endif
```

---

### **Problem 2: Missing Time Functions (clock_gettime, nanosleep)**

#### **Masalah:**
```
undefined reference to `clock_gettime'
undefined reference to `nanosleep'
```

#### **Penyebab:**
- Fungsi POSIX tidak ada di Windows API

#### **Solusi:**
Implementasi custom untuk Windows di `rate_limiter.c`:
```c
#ifdef _WIN32
int clock_gettime(int clk_id, struct timespec *tp) {
    FILETIME ft;
    ULARGE_INTEGER li;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    // Convert to timespec
    tp->tv_sec = (li.QuadPart - 116444736000000000ULL) / 10000000ULL;
    tp->tv_nsec = (li.QuadPart % 10000000ULL) * 100;
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    Sleep((DWORD)(req->tv_sec * 1000 + req->tv_nsec / 1000000));
    return 0;
}
#endif
```

---

### **Problem 3: Missing getopt() pada Windows**

#### **Masalah:**
```
undefined reference to `getopt'
```

#### **Penyebab:**
- getopt() adalah GNU extension, tidak ada di MinGW default

#### **Solusi:**
Stub implementation di `main.c`:
```c
#ifdef _WIN32
int getopt(int argc, char *const argv[], const char *optstring) {
    return -1; // Stub untuk sementara
}
#endif
```

---

### **Problem 4: GCC Tidak Membuat Executable**

#### **Masalah:**
```
gcc compilation success but no .exe file created
```

#### **Penyebab:**
- **Windows Defender** memblokir pembuatan executable
- GCC 15.2.0 dari MinGW-Builds memiliki masalah kompatibilitas
- Path dengan Unicode characters (ドキュメント) menyebabkan issues

#### **Diagnosis:**
Menggunakan `diagnose.bat`:
```batch
# Hasil diagnosis:
- RealTimeProtectionEnabled: True
- IoavProtectionEnabled: True
- GCC found but test.exe NOT created
- Kesimpulan: Windows Defender blocking
```

#### **Solusi:**
1. **Install MSYS2** (recommended):
   ```powershell
   # Download dari https://www.msys2.org/
   # Install development tools:
   pacman -S mingw-w64-ucrt-x86_64-gcc
   pacman -S mingw-w64-ucrt-x86_64-make
   ```

2. **Atau disable Windows Defender sementara** (not recommended)

3. **Atau compile di MSYS2 terminal**:
   ```bash
   cd "/c/Users/DELL/OneDrive/ドキュメント/ALL in ONE/TOOLS/c-secforce"
   mingw32-make clean
   mingw32-make all
   ```

---

### **Problem 5: Make Command Not Found di PowerShell**

#### **Masalah:**
```
make : The term 'make' is not recognized as the name of a cmdlet
```

#### **Penyebab:**
- `make` tidak tersedia di Windows PowerShell secara default
- Perlu menggunakan MSYS2 atau MinGW make

#### **Solusi:**
Gunakan salah satu:

1. **Build script** (build_msys2.bat):
   ```batch
   .\build_msys2.bat
   ```

2. **Direct MSYS2 make**:
   ```powershell
   C:\msys64\ucrt64\bin\mingw32-make.exe all
   ```

3. **MSYS2 Terminal**:
   ```bash
   mingw32-make all
   ```

---

### **Problem 6: Unicode Path Issues**

#### **Masalah:**
```
mingw32-make: *** readdir .: Invalid argument.  Stop.
```

#### **Penyebab:**
- Path mengandung karakter Jepang (ドキュメント)
- MinGW make kesulitan membaca directory dengan Unicode

#### **Solusi:**
1. Gunakan MSYS2 terminal langsung (native Unicode support)
2. Atau compile simple version tanpa make:
   ```bash
   gcc -o cyberforce.exe simple_cyberforce.c -O2 -Wall
   ```

---

## 🚀 Instalasi

### Windows (MSYS2 - Recommended)

1. **Install MSYS2**
   ```powershell
   # Download: https://www.msys2.org/
   # Jalankan installer dan ikuti wizard
   ```

2. **Update System**
   ```bash
   # Buka MSYS2 UCRT64 terminal
   pacman -Syu
   # Restart terminal jika diminta
   pacman -Su
   ```

3. **Install Development Tools**
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc
   pacman -S mingw-w64-ucrt-x86_64-make
   pacman -S mingw-w64-ucrt-x86_64-curl
   pacman -S mingw-w64-ucrt-x86_64-libssh
   pacman -S mingw-w64-ucrt-x86_64-openssl
   ```

4. **Clone/Copy Project**
   ```bash
   cd /c/Users/DELL/OneDrive/ドキュメント/ALL\ in\ ONE/TOOLS/
   # Project sudah ada di c-secforce/
   ```

### Linux

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential libcurl4-openssl-dev libssh-dev libssl-dev

# Fedora/RHEL
sudo dnf install gcc make libcurl-devel libssh-devel openssl-devel

# Arch Linux
sudo pacman -S base-devel curl libssh openssl
```

---

## 🔨 Kompilasi

### Metode 1: Menggunakan Batch Script (Windows - Termudah)

```powershell
# Compile simple version (tanpa external dependencies)
.\quick_compile.bat
```

### Metode 2: Menggunakan Makefile (MSYS2 Terminal)

```bash
# Di MSYS2 UCRT64 terminal
cd "/c/Users/DELL/OneDrive/ドキュメント/ALL in ONE/TOOLS/c-secforce"

# Build semua
mingw32-make clean
mingw32-make all

# Atau gunakan VS Code task
# Ctrl+Shift+B → Build CyberForce
```

### Metode 3: Manual Compilation

**Simple Version (Standalone - No dependencies):**
```bash
gcc -o cyberforce.exe simple_cyberforce.c -O2 -Wall
```

**Full Version (Dengan semua modules):**
```bash
gcc -o cyberforce.exe \
    src/main.c \
    src/core/attack_engine.c \
    src/core/protocol_handlers.c \
    src/core/rate_limiter.c \
    src/core/thread_manager.c \
    src/modules/http_brute.c \
    src/modules/ftp_brute.c \
    src/modules/ssh_brute.c \
    src/modules/mysql_brute.c \
    src/utils/logger.c \
    src/utils/wordlist_gen.c \
    src/utils/pattern_matcher.c \
    src/utils/proxy_rotator.c \
    src/utils/crypto_helper.c \
    -I./src/include \
    -lcurl -lssh -lssl -lcrypto -lpthread -lws2_32 \
    -O2 -Wall
```

---

## 📖 Cara Penggunaan

### Basic Commands

```bash
# Tampilkan bantuan
.\cyberforce.exe --help

# Tampilkan versi
.\cyberforce.exe --version

# HTTP Basic Auth Brute Force
.\cyberforce.exe -t http://example.com/admin -u users.txt -p passwords.txt -T 10

# FTP Brute Force
.\cyberforce.exe -t ftp://192.168.1.100 -u users.txt -p passwords.txt -T 5

# SSH Brute Force
.\cyberforce.exe -t ssh://192.168.1.100:22 -u users.txt -p passwords.txt -T 3

# MySQL Brute Force
.\cyberforce.exe -t mysql://192.168.1.100:3306 -u users.txt -p passwords.txt
```

### Advanced Options

```bash
# Dengan rate limiting (100 requests/second)
.\cyberforce.exe -t http://example.com -u users.txt -p pass.txt -r 100

# Dengan proxy
.\cyberforce.exe -t http://example.com -u users.txt -p pass.txt --proxy http://proxy:8080

# Verbose logging
.\cyberforce.exe -t http://example.com -u users.txt -p pass.txt -v

# Export results ke JSON
.\cyberforce.exe -t http://example.com -u users.txt -p pass.txt -o results.json
```

### Parameter Reference

| Parameter | Deskripsi | Default |
|-----------|-----------|---------|
| `-t, --target` | Target URL (http/ftp/ssh/mysql) | Required |
| `-u, --users` | File berisi username list | Required |
| `-p, --passwords` | File berisi password list | Required |
| `-T, --threads` | Jumlah threads parallel | 10 |
| `-r, --rate` | Rate limit (requests/second) | 50 |
| `-v, --verbose` | Verbose logging | Off |
| `-o, --output` | Output file untuk results | stdout |
| `--proxy` | Proxy server URL | None |
| `--timeout` | Connection timeout (seconds) | 10 |
| `--help` | Tampilkan bantuan | - |
| `--version` | Tampilkan versi | - |

---

## 📦 Dependensi

### Runtime Dependencies

- **libcurl** (>= 7.68.0) - HTTP/HTTPS requests
- **libssh** (>= 0.9.0) - SSH protocol support
- **OpenSSL** (>= 1.1.1) - Cryptographic operations
- **pthread** - Multi-threading support
- **Winsock2** (Windows only) - Network socket API

### Build Dependencies

- **GCC** (>= 8.0) atau **Clang** (>= 10.0)
- **Make** (>= 4.0)
- Development headers untuk semua runtime dependencies

---

## ⚖️ Legal & Ethical Use

**⚠️ DISCLAIMER:**

Tools ini dibuat untuk **TUJUAN EDUKASI** dan **AUTHORIZED SECURITY TESTING** saja. 

**DILARANG menggunakan tools ini untuk:**
- Unauthorized access ke sistem orang lain
- Melanggar hukum komputer/cybercrime
- Merusak atau mengakses data tanpa izin
- Aktivitas ilegal lainnya

**Pengguna bertanggung jawab penuh** atas penggunaan tools ini. Developer tidak bertanggung jawab atas penyalahgunaan.

**Selalu dapatkan izin tertulis** sebelum melakukan penetration testing pada sistem apapun.

---

## 🛡️ Security Considerations

- Tools ini meninggalkan jejak di log server target
- Rate limiting membantu tapi tidak menghilangkan deteksi
- Gunakan proxy/VPN untuk anonymitas tambahan
- Pastikan testing dilakukan di environment isolated

---

## 📝 License

MIT License - See LICENSE file for details

---

## 👥 Kontribusi

Contributions welcome! Silakan submit pull request atau buka issue untuk bug reports.

---

## 📧 Kontak & Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/c-secforce/issues)
- **Documentation**: [Wiki](https://github.com/yourusername/c-secforce/wiki)

---

## 🎯 Roadmap

- [ ] GUI Interface
- [ ] Web dashboard untuk monitoring
- [ ] Support untuk protokol tambahan (RDP, SMB, LDAP)
- [ ] Machine learning untuk pattern detection
- [ ] Distributed attack coordination
- [ ] Better evasion techniques

---

**Made with ❤️ for Security Researchers**

