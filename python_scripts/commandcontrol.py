'''
Created on Nov 25, 2025

@author: tajja
'''
import serial
import time

# Replace with your gateway COM port and baud rate
COM_PORT = 'COM5'      # Example: 'COM3' on Windows, '/dev/ttyUSB0' on Linux
BAUD_RATE = 115200

# Open serial connection
ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Allow board to reset after opening serial

# Function for sending input data
def send_command(cmd):
    """Send a single-character command to the gateway node."""
    ser.write(cmd.encode('utf-8'))  # Send command as bytes
    ser.flush()
    print(f"Sent command: {cmd}")

try:
    while True:
        cmd = input("Enter command to send to node (e.g., 'R'): ").strip().upper()
        if len(cmd) == 1:  # Only allow single-character commands
            send_command(cmd)
        else:
            print("Invalid input! Type a single character command.")
except KeyboardInterrupt:
    print("\nExiting...")
finally:
    ser.close()