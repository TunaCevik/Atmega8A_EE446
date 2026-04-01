import serial
import time
import sys

# --- Configuration ---
# Change this to match the port in your Windows Device Manager
SERIAL_PORT = "COM11" 
BAUD_RATE = 9600

def monitor_serial():
    print(f"Attempting to connect to {SERIAL_PORT} at {BAUD_RATE} baud...")
    
    try:
        # 1. Open the connection
        # timeout=1 ensures readline() doesn't freeze the program forever if no data arrives
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            print("Connection successful! Listening for data... (Press Ctrl+C to exit)\n")
            print("-" * 50)
            
            # 2. The Infinite Listening Loop
            while True:
                # Read raw bytes until a newline character (\n) is received
                raw_bytes = ser.readline()
                
                # If we actually received bytes (not just a timeout)
                if raw_bytes:
                    try:
                        # Decode bytes to a string, ignore garbage electrical noise
                        data_string = raw_bytes.decode('utf-8', errors='ignore').strip()
                        
                        # Print only if the string is not empty
                        if data_string:
                            print(f"[MCU]: {data_string}")
                            
                    except Exception as decode_err:
                        print(f"[Warning] Decode error: {decode_err}")
                
    except serial.SerialException as e:
        # This triggers if the port doesn't exist, or if the USB is suddenly unplugged
        print(f"\n[CRITICAL] Hardware connection error on {SERIAL_PORT}.")
        print(f"Details: {e}")
        print("Please check your USB cable and ensure no other program (like Arduino IDE) is using the port.")
        
    except KeyboardInterrupt:
        # This catches the user pressing Ctrl+C to exit the program cleanly
        print("\n[INFO] Serial monitor closed by user.")
        sys.exit(0)

if __name__ == "__main__":
    monitor_serial()