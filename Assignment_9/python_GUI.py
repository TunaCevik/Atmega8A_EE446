import tkinter as tk
from tkinter import messagebox
from pymodbus.client import ModbusSerialClient
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import threading
import time

COM_PORT = 'COM11' 
BAUD_RATE = 19200
SLAVE_ID = 2

client = ModbusSerialClient(port=COM_PORT, baudrate=BAUD_RATE, timeout=1)

class PIDApp:
    def __init__(self, root):
        self.root = root
        self.root.title("MODBUS PID GUI")
        self.root.geometry("800x600")

        # Data arrays for graphing
        self.time_data = []
        self.real_temp_data = []
        self.set_temp_data = []
        self.sample_count = 0

        self.setup_ui()
        self.connect_modbus()

        # Start the background polling loop (1 second updates)
        self.running = True
        self.poll_thread = threading.Thread(target=self.poll_data, daemon=True)
        self.poll_thread.start()

    def connect_modbus(self):
        if not client.connect():
            messagebox.showerror("Connection Error", f"Failed to connect to {COM_PORT}")

    def setup_ui(self):
        # --- Top Controls Frame ---
        control_frame = tk.Frame(self.root)
        control_frame.pack(side=tk.TOP, fill=tk.X, padx=20, pady=20)

        # Kp, Ki, Kd
        tk.Label(control_frame, text="Kp").grid(row=0, column=0, padx=5)
        self.kp_entry = tk.Entry(control_frame, width=8)
        self.kp_entry.grid(row=0, column=1, padx=5)
        
        tk.Label(control_frame, text="Ki").grid(row=0, column=2, padx=5)
        self.ki_entry = tk.Entry(control_frame, width=8)
        self.ki_entry.grid(row=0, column=3, padx=5)
        
        tk.Label(control_frame, text="Kd").grid(row=0, column=4, padx=5)
        self.kd_entry = tk.Entry(control_frame, width=8)
        self.kd_entry.grid(row=0, column=5, padx=5)

        # Set Value & Current Temp
        tk.Label(control_frame, text="Set Value").grid(row=0, column=6, padx=10)
        self.set_entry = tk.Entry(control_frame, width=8)
        self.set_entry.grid(row=0, column=7, padx=5)

        tk.Label(control_frame, text="Current Temp").grid(row=0, column=8, padx=10)
        self.temp_var = tk.StringVar(value="--")
        self.temp_label = tk.Label(control_frame, textvariable=self.temp_var, width=8, bg="white", relief="sunken")
        self.temp_label.grid(row=0, column=9, padx=5)

        # Buttons
        tk.Button(control_frame, text="Write PID Parameters", bg="lightblue", command=self.write_pid).grid(row=1, column=0, columnspan=6, pady=10, sticky="ew")
        tk.Button(control_frame, text="Write Set Value", bg="lightblue", command=self.write_set_value).grid(row=1, column=6, columnspan=2, pady=10, sticky="ew")

        # Status Label
        self.status_label = tk.Label(self.root, text="", fg="green")
        self.status_label.pack(anchor="w", padx=20)

        # --- Graphing Frame ---
        self.fig = Figure(figsize=(7, 4), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_xlabel("Time (samples)")
        self.ax.set_ylabel("Temperature (°C)")
        self.ax.set_xlim(0, 50)
        self.ax.set_ylim(0, 100)
        
        self.line_real, = self.ax.plot([], [], 'b-', label="Real Temp")
        self.line_set, = self.ax.plot([], [], 'r-', label="Set Temp")
        self.ax.legend(loc="upper right")

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True, padx=20, pady=10)

    def write_pid(self):
        try:
            kp = int(self.kp_entry.get())
            ki = int(self.ki_entry.get())
            kd = int(self.kd_entry.get())
            
            client.write_register(1, kp, device_id=SLAVE_ID)
            client.write_register(2, ki, device_id=SLAVE_ID)
            client.write_register(3, kd, device_id=SLAVE_ID)
            self.status_label.config(text="PID Parameters Written", fg="green")
        except ValueError:
            self.status_label.config(text="Error: Enter valid integers", fg="red")

    def write_set_value(self):
        try:
            sv = int(self.set_entry.get())
            client.write_register(0, sv, device_id=SLAVE_ID)
            self.status_label.config(text="Set Value Written", fg="green")
        except ValueError:
            self.status_label.config(text="Error: Enter a valid integer", fg="red")

    def poll_data(self):
        while self.running:
            try:
                # Read 5 holding registers starting at address 0
                result = client.read_holding_registers(address=0, count=5, device_id=SLAVE_ID)
                if not result.isError():
                    set_temp = result.registers[0]
                    current_temp = result.registers[4]

                    # Update GUI string
                    self.temp_var.set(str(current_temp))

                    # Update Data Arrays
                    self.time_data.append(self.sample_count)
                    self.set_temp_data.append(set_temp)
                    self.real_temp_data.append(current_temp)
                    self.sample_count += 1

                    # Keep graph window scrolling if samples exceed 50
                    if self.sample_count > 50:
                        self.ax.set_xlim(self.sample_count - 50, self.sample_count)

                    # Update plot
                    self.line_real.set_data(self.time_data, self.real_temp_data)
                    self.line_set.set_data(self.time_data, self.set_temp_data)
                    self.canvas.draw_idle()
            except Exception as e:
                print(f"Polling error: {e}")
            
            # The graph must update every one second [cite: 22]
            time.sleep(1)

    def on_closing(self):
        self.running = False
        client.close()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = PIDApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()