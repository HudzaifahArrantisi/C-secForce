# CyberForce Build Script for Windows PowerShell
param(
    [string]$Action = "build",
    [string]$Mode = "full"
)

$ErrorActionPreference = "Continue"
if (Test-Path "C:\msys64\ucrt64\bin") {
    $env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
}
$DefaultMsysGcc = "C:\msys64\ucrt64\bin\gcc.exe"
$CC = if (Test-Path $DefaultMsysGcc) { $DefaultMsysGcc } else { "gcc" }
$Target = "cyberforce.exe"
$SimpleSource = "simple_cyberforce.c"
$FullCFlags = @("-Wall", "-Wextra", "-O2", "-D_WIN32", "-DHAS_CURL", "-DCURL_STATICLIB", "-Isrc", "-Isrc/include")
$FullLibs = @("-lcurl", "-lssh", "-lssl", "-lcrypto", "-lws2_32", "-lm", "-lpthread")
$FullSources = @(
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
    Write-Host "CyberForce v2.0.0" -ForegroundColor Cyan
    Write-Host "Security Testing Framework" -ForegroundColor Cyan
    Write-Host ""
}

function Explain-CompilerFailure {
    param([int]$ExitCode)

    Write-Host ""
    Write-Host "ERROR: Compiler failed with exit code $ExitCode" -ForegroundColor Red
    Write-Host "GCC could not compile a minimal C program, so this is a local compiler/toolchain problem before the project build starts." -ForegroundColor Yellow
    Write-Host "Install or repair MSYS2/MinGW-w64, then open a new terminal and run this script again." -ForegroundColor Yellow
    Write-Host "Recommended packages:" -ForegroundColor Yellow
    Write-Host "  pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-libssh mingw-w64-ucrt-x86_64-openssl mingw-w64-ucrt-x86_64-pthreads"
    if ($ExitCode -eq -1073741511) {
        Write-Host "The raw Windows exit code indicates a missing or incompatible runtime DLL." -ForegroundColor Yellow
    }
}

function Test-Compiler {
    $tempDir = Join-Path $env:TEMP "cyberforce-build-test"
    New-Item -ItemType Directory -Path $tempDir -Force | Out-Null
    $testC = Join-Path $tempDir "compiler_test.c"
    $testExe = Join-Path $tempDir "compiler_test.exe"

    Set-Content -Path $testC -Value 'int main(void) { return 0; }' -Encoding ASCII
    & $CC $testC "-o" $testExe 2>&1 | Out-Null
    $exit = $LASTEXITCODE
    $created = Test-Path $testExe

    Remove-Item $testC -Force -ErrorAction SilentlyContinue
    Remove-Item $testExe -Force -ErrorAction SilentlyContinue

    if ($exit -ne 0 -or -not $created) {
        if ($exit -eq 0) { $exit = 1 }
        Explain-CompilerFailure $exit
        return $false
    }

    return $true
}

function Clean-Build {
    Write-Host "[*] Cleaning build files..." -ForegroundColor Yellow
    Get-ChildItem -Path . -Filter "*.o" -Recurse | Remove-Item -Force -ErrorAction SilentlyContinue
    Remove-Item $Target -Force -ErrorAction SilentlyContinue
    Remove-Item "cyberforce_simple.exe" -Force -ErrorAction SilentlyContinue
    Write-Host "[+] Clean completed" -ForegroundColor Green
}

function Build-Simple {
    Show-Banner
    Write-Host "[*] Building standalone version..." -ForegroundColor Yellow

    if (-not (Test-Compiler)) { return $false }

    & $CC "-Wall" "-Wextra" "-O2" $SimpleSource "-o" $Target 2>&1
    if ($LASTEXITCODE -ne 0) {
        Explain-CompilerFailure $LASTEXITCODE
        return $false
    }

    Write-Host "[+] Build successful: $Target" -ForegroundColor Green
    return $true
}

function Build-Full {
    Show-Banner
    Write-Host "[*] Building full version with external libraries..." -ForegroundColor Yellow

    if (-not (Test-Compiler)) { return $false }

    $objects = @()
    foreach ($source in $FullSources) {
        if (-not (Test-Path $source)) {
            Write-Host "ERROR: Source file not found: $source" -ForegroundColor Red
            return $false
        }

        $object = $source -replace '\.c$', '.o'
        $objects += $object
        Write-Host "[*] Compiling $source" -ForegroundColor Blue
        & $CC @FullCFlags "-c" $source "-o" $object 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Compilation failed for $source" -ForegroundColor Red
            return $false
        }
    }

    Write-Host "[*] Linking $Target" -ForegroundColor Yellow
    & $CC "-o" $Target @objects @FullLibs 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Linking failed. Install libcurl, libssh, OpenSSL, and pthreads for your MinGW environment." -ForegroundColor Red
        return $false
    }

    Write-Host "[+] Build successful: $Target" -ForegroundColor Green
    return $true
}

function Run-CyberForce {
    if (-not (Test-Path $Target)) {
        if (-not (Build-Full)) { return }
    }

    Write-Host ""
    Write-Host "[*] Running $Target --help" -ForegroundColor Yellow
    & ".\$Target" "--help"
}

function Show-Diagnose {
    Show-Banner
    Write-Host "Project path: $PWD"
    Write-Host "Compiler path:"
    where.exe $CC
    Write-Host ""
    Write-Host "Compiler version:"
    & $CC "--version"
    Write-Host ""
    Test-Compiler | Out-Null
}

function Show-Help {
    Show-Banner
    Write-Host "Usage:"
    Write-Host "  .\build.ps1 build             Build full executable with libcurl/libssh/OpenSSL"
    Write-Host "  .\build.ps1 build simple      Build standalone executable"
    Write-Host "  .\build.ps1 run               Build standalone executable if needed, then show help"
    Write-Host "  .\build.ps1 clean             Remove build outputs"
    Write-Host "  .\build.ps1 diagnose          Check compiler health"
    Write-Host ""
    Write-Host "For normal first run, use:"
    Write-Host "  .\build.ps1 run"
}

switch ($Action.ToLower()) {
    "clean" { Clean-Build }
    "build" {
        $ok = if ($Mode.ToLower() -eq "simple") { Build-Simple } else { Build-Full }
        if (-not $ok) { exit 1 }
    }
    "run" { Run-CyberForce }
    "diagnose" { Show-Diagnose }
    "help" { Show-Help }
    default {
        Write-Host "ERROR: Unknown action: $Action" -ForegroundColor Red
        Show-Help
    }
}
