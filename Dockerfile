FROM ubuntu:22.04

LABEL maintainer="cybersecurity-student@university.edu"
LABEL version="2.0.0"
LABEL description="CyberForce - Advanced Security Testing Tool"

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libcurl4-openssl-dev \
    libssl-dev \
    libssh-dev \
    git \
    curl \
    wget \
    tor \
    proxychains \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /cyberforce

# Copy source code
COPY . .

# Build CyberForce
RUN make release

# Create directories
RUN mkdir -p /data/wordlists /data/output /data/logs

# Configure Tor
RUN echo "SocksPort 9050" >> /etc/tor/torrc
RUN echo "ControlPort 9051" >> /etc/tor/torrc
RUN echo "CookieAuthentication 1" >> /etc/tor/torrc

# Set up proxychains
RUN echo "socks5 127.0.0.1 9050" >> /etc/proxychains.conf

# Create non-root user
RUN useradd -m -s /bin/bash cyberuser && \
    chown -R cyberuser:cyberuser /cyberforce /data

USER cyberuser

# Volume for wordlists and results
VOLUME ["/data"]

# Default command
ENTRYPOINT ["/cyberforce/cyberforce"]
CMD ["--help"]