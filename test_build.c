#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                 CYBERFORCE v2.0.0                        ║\n");
    printf("║         Advanced Security Testing Framework             ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        printf("CyberForce - Security Testing Framework\n\n");
        printf("Usage: cyberforce [OPTIONS]\n\n");
        printf("Required:\n");
        printf("  -t, --target HOST          Target host or URL\n");
        printf("  -U, --user FILE/USER       Username or userlist file\n");
        printf("  -P, --pass FILE/PASS       Password or passlist file\n\n");
        printf("Modules:\n");
        printf("  -m, --module MODULE        Attack module (http, ftp, ssh, mysql)\n");
        printf("  -p, --port PORT            Port number\n");
        printf("  -o, --output FILE          Output file\n");
        printf("  -v, --verbose LEVEL        Verbosity (0-5)\n\n");
        printf("Note: Full version requires libcurl, libssh, and openssl\n");
        printf("      This is a test build to verify Windows compilation works\n\n");
        return 0;
    }
    
    printf("[!] This is a test build\n");
    printf("[!] Full version requires additional libraries:\n");
    printf("    - libcurl (for HTTP module)\n");
    printf("    - libssh (for SSH module)\n");
    printf("    - openssl (for crypto functions)\n\n");
    printf("To install dependencies (MSYS2):\n");
    printf("  pacman -S mingw-w64-x86_64-curl\n");
    printf("  pacman -S mingw-w64-x86_64-libssh\n");
    printf("  pacman -S mingw-w64-x86_64-openssl\n\n");
    printf("Run with --help for usage information\n\n");
    
    return 0;
}
