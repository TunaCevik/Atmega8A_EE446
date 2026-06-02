import serial
from tkinter import *
import tkinter as tk
from serial.serialutil import SerialException

root = Tk()
root.title("MODBUS GUI")
root.geometry("500x500")

# Modbus communication parameters
port_name = 'COM11'
baudrate = 19200
parity = serial.PARITY_NONE
stopbits = serial.STOPBITS_ONE
bytesize = serial.EIGHTBITS
timeout = 1
register_address = 100#3105
slave_address = 2

# serial connection setup fuction
def configure_serial_connection(port, baudrate, parity, stopbits, bytesize, timeout):
    try:
        ser = serial.Serial(
            port=port, 
            baudrate=baudrate, 
            parity=parity, 
            stopbits=stopbits, 
            bytesize=bytesize, 
            timeout=timeout
        )
        return ser
    except SerialException as e:
        print(f"Connection Error: {e}")
        status_label.config(text="Connection Error: Port not open.", fg="red")
        return None

# CRC calculation function
def calculate_crc(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc.to_bytes(2, byteorder='little')

# Modbus request sending and responce function
def send_modbus_request(ser, request, response_length):
    try:
        ser.write(request)
        response = ser.read(response_length)  
        return response
    except SerialException as e:
        print(f"Data Sending Error: {e}")
        status_label.config(text="Data Sending Error.", fg="red")
        return None

# Holding register reading function
def read_holding_register(port, baudrate, parity, stopbits, bytesize, timeout, register_address, slave_address):
    read_display_text.delete('1.0', END)
    ser = configure_serial_connection(port, baudrate, parity, stopbits, bytesize, timeout)
    
    if ser is None:
        return

    read_request = bytearray([slave_address, 0x03, (register_address >> 8) & 0xFF, register_address & 0xFF, 0x00, 0x01])
    read_request += calculate_crc(read_request)
    read_response = send_modbus_request(ser, read_request, 7)  # 7 baytlık response waiting
    
    if read_response is None:
        return

    if len(read_response) >= 5 and read_response[0] == slave_address and read_response[1] == 0x03:
        byte_count = read_response[2]
        if byte_count == 2:
            lsp_value = int.from_bytes(read_response[3:5], byteorder='big')
            read_display_text.insert(tk.END, lsp_value)
            print("Holding Register Value:", lsp_value)
            status_label.config(text="Reading Success", fg="green")
        else:
            print("Invalid Response: Unexpected byte count")
            status_label.config(text="Invalid Response: Unexpected byte count", fg="red")
    else:
        print("Invalid Response")
        status_label.config(text="Invalid Response.", fg="red")

    ser.close()

# Holding register write fuction
def write_holding_register(port, baudrate, parity, stopbits, bytesize, timeout, register_address, slave_address, value):
    ser = configure_serial_connection(port, baudrate, parity, stopbits, bytesize, timeout)
    
    if ser is None:
        return

    write_request = bytearray([slave_address, 0x06, (register_address >> 8) & 0xFF, register_address & 0xFF,
                               (value >> 8) & 0xFF, value & 0xFF])
    write_request += calculate_crc(write_request)
    response = send_modbus_request(ser, write_request, 8)  

    if response is None:
        return

    if len(response) >= 5 and response[0] == slave_address and response[1] == 0x06:
        print("Holding Register Writing Success:", value)
        status_label.config(text="Writing Success", fg="green")
    else:
        print("Invalid Response")
        status_label.config(text="Invalid Response.", fg="red")
    
    ser.close()

# GUI description and locate in the form
write_value_text = Text(root, width=13, height=1, font=("Helvetica", 16))
write_value_text.place(relx=0.6, rely=0.4, relheight=0.05, relwidth=0.2)

read_holding_btn = Button(root, text="Read Holding Register", command=lambda: read_holding_register(port_name, baudrate, parity, stopbits, bytesize, timeout, register_address, slave_address), bg='yellow')
read_holding_btn.place(relx=0.2, rely=0.5, relwidth=0.3, relheight=0.05)

read_display_text = Text(root, width=13, height=1, font=("Helvetica", 16))
read_display_text.place(relx=0.6, rely=0.5, relheight=0.05, relwidth=0.2)

write_hol_btn = Button(root, text="Write Holding Register", command=lambda: write_holding_register(port_name, baudrate, parity, stopbits, bytesize, timeout, register_address, slave_address, int(write_value_text.get("1.0", tk.END).strip())), bg='yellow')
write_hol_btn.place(relx=0.2, rely=0.4, relwidth=0.3, relheight=0.05)

status_label = Label(root, text="Connection Status: Unknown", fg="blue")
status_label.place(relx=0.1, rely=0.9)

root.mainloop()

