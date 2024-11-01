#!/bin/bash

# capture_client_any.sh
# Script to capture UDP and TCP traffic on all interfaces (client side)

# ===========================
# Configuration Section
# ===========================

# Use the 'any' pseudo-interface to capture on all interfaces
INTERFACE="any"

# Ports to capture
UDP_PORT=7000
TCP_PORT=9999

# Server IP Address (for reference)
SERVER_IP="129.108.156.68"

# Output directory for captures
OUTPUT_DIR="$HOME/tcpdump_captures"
mkdir -p "$OUTPUT_DIR"

# Timestamp for the capture fileudp.port == 7000 || tcp.port == 9999

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Output capture file name
OUTPUT_FILE="$OUTPUT_DIR/client_capture_$TIMESTAMP.pcap"

# ===========================
# Start `tcpdump` Capture
# ===========================

echo "Starting tcpdump on interface $INTERFACE..."
echo "Capturing UDP port $UDP_PORT and TCP port $TCP_PORT"
echo "Output file: $OUTPUT_FILE"

# Start tcpdump in the background
sudo tcpdump -i "$INTERFACE" -w "$OUTPUT_FILE" "(udp port $UDP_PORT) or (tcp port $TCP_PORT)" &
TCPDUMP_PID=$!

# Function to stop tcpdump on script exit
cleanup() {
    echo "Stopping tcpdump (PID: $TCPDUMP_PID)..."
    sudo kill "$TCPDUMP_PID"
    wait "$TCPDUMP_PID" 2>/dev/null
    echo "Capture saved to $OUTPUT_FILE"
    exit
}

# Trap script termination to ensure tcpdump is stopped
trap cleanup SIGINT SIGTERM

# ===========================
# Run Client Toolchain Commands
# ===========================

echo "Running client toolchain commands..."

# Start tunnel_udp_over_tcp_client in the background
./tunnel_udp_over_tcp_client 7000 "$SERVER_IP" 9999 &
TUNNEL_CLIENT_PID=$!

# Start send_receive_udp in the background
./send_receive_udp localhost 7000 &
SEND_RECEIVE_PID=$!

# Wait for both client processes to complete
wait "$TUNNEL_CLIENT_PID"
wait "$SEND_RECEIVE_PID"

# ===========================
# Cleanup
# ===========================

cleanup
