#!/bin/bash
# Test HTTP brute force against local test server

echo "Starting HTTP brute force test..."

# Create test server
python3 -m http.server 8080 &
SERVER_PID=$!
sleep 2

# Run CyberForce
./cyberforce -t http://localhost:8080 \
             -U data/wordlists/common_users.txt \
             -P data/wordlists/common_passwords.txt \
             -m http \
             -T 5 \
             --timeout 5 \
             --delay 10 \
             -o test_results.txt \
             -v 2

# Cleanup
kill $SERVER_PID
echo "Test completed!"