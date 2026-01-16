# CyberForce Build Script for Windows PowerShell
param([string]$Action = "build")

$ErrorActionPreference = "Continue"
$CC = "gcc"
$CFLAGS = "-Wall", "-Wextra", "-O2", "-D_WIN32", "-DCURL_STATICLIB", "-Isrc/include"
$LIBS = "-lcurl", "-lssh", "-lssl", "-lcrypto", "-lws2_32", "-lm", "-lpthread"
$TARGET = "cyberforce.exe"

$SOURCES = @(
    "src/main.c",
    "src/core/attack_engine.c",
    "src/core/thread_manager.c",
    "src/core/rate_limiter.c",
    "src/core/protocol_handlers.c",
    "src/modules/http_brute.c",
    "src/modules/ftp_brute.c",
    "src/modules/ssh_brute.c",
    "src/modules/mysql_brute.c",
    "src/utils/wordlist_gen.c",
    "src/utils/pattern_matcher.c",
    "src/utils/proxy_rotator.c",
    "src/utils/logger.c",
    "src/utils/crypto_helper.c"
)

function Show-Banner {
    Write-Host ""
    Write-Host "╔══════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║                 CYBERFORCE v2.0.0                        ║" -ForegroundColor Cyan
    Write-Host "║         Advanced Security Testing Framework             ║" -ForegroundColor Cyan
    Write-Host "╚══════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""
}

function Clean-Build {
    Write-Host "[*] Cleaning build files..." -ForegroundColor Yellow
    Get-ChildItem -Path . -Filter "*.o" -Recurse | Remove-Item -Force -ErrorAction SilentlyContinue
    if (Test-Path $TARGET) { Remove-Item $TARGET -Force }
    Get-ChildItem -Path . -Filter "*.log" | Remove-Item -Force -ErrorAction SilentlyContinue
    Get-ChildItem -Path . -Filter "*.checkpoint" | Remove-Item -Force -ErrorAction SilentlyContinue
    Write-Host "[+] Clean completed!" -ForegroundColor Green
}

function Build-Project {
    Show-Banner
    Write-Host "[*] Building CyberForce..." -ForegroundColor Yellow
    Write-Host ""
    
    $objects = @()
    $failed = $false
    
    foreach ($source in $SOURCES) {
        if (-not (Test-Path $source)) {
            Write-Host "ERROR: Source file not found: $source" -ForegroundColor Red
            $failed = $true
            continue
        }
        
        $objFile = $source -replace '\.c$', '.o'
        $objects += $objFile
        
        Write-Host "[*] Compiling $source..." -ForegroundColor Blue
        
        $compileCmd = "$CC $($CFLAGS -join ' ') -c `"$source`" -o `"$objFile`""
        Invoke-Expression $compileCmd
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Compilation failed for $source" -ForegroundColor Red
            $failed = $true
            break
        }
    }
    
    if ($failed) {
        Write-Host ""
        Write-Host "ERROR: Build failed" -ForegroundColor Red
        return $false
    }
    
    Write-Host ""
    Write-Host "[*] Linking $TARGET..." -ForegroundColor Yellow
    
    $linkCmd = "$CC -o `"$TARGET`" $($objects -join ' ') $($LIBS -join ' ')"
    Invoke-Expression $linkCmd
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Linking failed" -ForegroundColor Red
        return $false
    }
    
    Write-Host ""
    Write-Host "[+] Build successful!" -ForegroundColor Green
    Write-Host "[+] Executable: $TARGET" -ForegroundColor Green
    return $true
}

function Run-CyberForce {
    if (-not (Test-Path $TARGET)) {
        Write-Host "Executable not found. Building first..." -ForegroundColor Yellow
        if (-not (Build-Project)) { return }
    }
    
    Write-Host ""
    Write-Host "[*] Running CyberForce..." -ForegroundColor Yellow
    Write-Host ""
    
    & ".\$TARGET" --help
}

function Show-Help {
    Show-Banner
    Write-Host "Build Script Usage:" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  .\build.ps1 build   - Compile the project" -ForegroundColor White
    Write-Host "  .\build.ps1 clean   - Clean build files" -ForegroundColor White
    Write-Host "  .\build.ps1 run     - Build and run with --help" -ForegroundColor White
    Write-Host "  .\build.ps1 help    - Show this help message" -ForegroundColor White
    Write-Host ""
    Write-Host "Requirements:" -ForegroundColor Cyan
    Write-Host "  - MinGW-w64 GCC compiler" -ForegroundColor White
    Write-Host "  - libcurl, libssh, openssl libraries" -ForegroundColor White
    Write-Host ""
}

switch ($Action.ToLower()) {
    "clean" { Clean-Build }
    "build" { Build-Project }
    "run" { Run-CyberForce }
    "help" { Show-Help }
    default {
        Write-Host "ERROR: Unknown action: $Action" -ForegroundColor Red
        Show-Help
    }
}
