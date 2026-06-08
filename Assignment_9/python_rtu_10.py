import time
import ssl
import paho.mqtt.client as mqtt
from pymodbus.client import ModbusSerialClient

# ==========================================
# 1. Configuration Constants
# ==========================================
# Modbus Settings
COM_PORT = 'COM11'
BAUD_RATE = 19200
SLAVE_ID = 2

# HiveMQ Settings (From your screenshot)
MQTT_BROKER = "12c0af29fee04d34bb004eef4275b013.s1.eu.hivemq.cloud"
MQTT_PORT = 8883
MQTT_TOPIC = "ee446/assignment2/temperature"

# TODO: Replace these with the credentials you make in Access Management
MQTT_USERNAME = "ee446_assignment_10"
MQTT_PASSWORD = "ee446_assignment_10_C"

# ==========================================
# 2. MQTT Callbacks
# ==========================================
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Successfully connected to HiveMQ Cloud!")
    else:
        print(f"[MQTT] Connection failed with code {rc}")

def on_publish(client, userdata, mid):
    print(f"[MQTT] Message {mid} published successfully.")

# ==========================================
# 3. Main Application Loop
# ==========================================
def main():
    # --- Initialize Modbus ---
    modbus_client = ModbusSerialClient(port=COM_PORT, baudrate=BAUD_RATE, timeout=1)
    if not modbus_client.connect():
        print(f"[ERROR] Failed to connect to Modbus on {COM_PORT}")
        return
    print(f"[MODBUS] Connected to {COM_PORT}")

    # --- Initialize MQTT ---
    mqtt_client = mqtt.Client(client_id="PythonGateway_EE446")
    mqtt_client.on_connect = on_connect
    mqtt_client.on_publish = on_publish
    
    # Enable secure TLS connection (Required for port 8883)
    mqtt_client.tls_set(tls_version=ssl.PROTOCOL_TLS)
    mqtt_client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    print("[MQTT] Attempting to connect to broker...")
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    
    # Start the network loop in the background
    mqtt_client.loop_start()

    # --- Polling Loop ---
    try:
        while True:
            # 1. Read Modbus Register
            # Temperature is stored in holding_register[4], which is address 4
            result = modbus_client.read_holding_registers(address=4, count=1, device_id=SLAVE_ID)
            
            if not result.isError():
                current_temp = result.registers[0]
                print(f"[LOCAL] Read Temperature: {current_temp} °C")
                
                # 2. Publish to MQTT
                payload = f'{{"temperature": {current_temp}, "unit": "C"}}'
                mqtt_client.publish(MQTT_TOPIC, payload, qos=1)
            else:
                print("[ERROR] Modbus read failed.")
            
            # The requirement specifies periodic publishing [cite: 62]
            time.sleep(2) 

    except KeyboardInterrupt:
        print("\n[SYSTEM] Shutting down...")
    finally:
        modbus_client.close()
        mqtt_client.loop_stop()
        mqtt_client.disconnect()

if __name__ == "__main__":
    main()