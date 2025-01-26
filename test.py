import socket

# Set up the UDP connection
udp_ip = "192.168.1.2"  # Replace with the ESP32-CAM's IP address
udp_port = 12345
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Send a test message
message = "50,100"  # Example coordinates
sock.sendto(message.encode(), (udp_ip, udp_port))

print(f"Sent message: {message}")
sock.close()
