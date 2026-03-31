import tkinter as tk
import serial
import threading

# ---------------------------------------------------------
# 1. Serial Port Configuration
# ---------------------------------------------------------
SERIAL_PORT = "COM11" 
BAUD_RATE = 9600      

try:
    serialConnection = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Successfully connected to {SERIAL_PORT} at {BAUD_RATE} baud.")
except serial.SerialException:
    print(f"Error: Could not open serial port {SERIAL_PORT}.")
    serialConnection = None

# ---------------------------------------------------------
# 2. Create Tkinter GUI Window
# ---------------------------------------------------------
root = tk.Tk()
root.title("AVR LDR Light Intensity Monitor")

text_box = tk.Text(root, height=15, width=50, bg="black", fg="green", font=("Consolas", 12))
text_box.pack(padx=10, pady=10)

# This function is safely called by the Main GUI Thread
def update_gui_text(text_data):
    text_box.insert(tk.END, f"{text_data}\n")
    text_box.see(tk.END)

# ---------------------------------------------------------
# 3. Serial Reading Thread
# ---------------------------------------------------------
def read_serial():
    while True:
        if serialConnection and serialConnection.is_open:
            try:
                # errors='ignore' prevents the crash if baud rates mismatch and we get garbage
                raw_bytes = serialConnection.readline()
                data = raw_bytes.decode('utf-8', errors='ignore').strip() 
                
                if data:
                    # Safely hand the data over to the Tkinter main loop
                    root.after(0, update_gui_text, data)
                    
            except Exception as e:
                # If an error happens, just print it to console but DO NOT break the loop!
                print(f"Serial read error: {e}")

# ---------------------------------------------------------
# 4. Start Application
# ---------------------------------------------------------
serial_thread = threading.Thread(target=read_serial, daemon=True)
serial_thread.start()

root.mainloop()

# Clean Up runs when the user closes the window
if serialConnection and serialConnection.is_open:
    serialConnection.close()
    print("Serial connection closed safely.")