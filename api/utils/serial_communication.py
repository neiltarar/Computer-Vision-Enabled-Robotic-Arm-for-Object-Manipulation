import serial
from time import sleep
from dotenv import load_dotenv
import os

load_dotenv()
machine = os.getenv("MACHINE_NAME")
port = "/dev/ttyUSB0" if machine == "pc" else "/dev/ttyS0"

def send_receive_serial_data(command, port="/dev/ttyUSB0", baudrate=9600):
    ser = serial.Serial(port, baudrate, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                        bytesize=serial.EIGHTBITS, timeout=0.1)

    while True:
        data = "#" + str(command) + "\n"
        ser.write(data.encode())
        print("Sent data:", data)
        sleep(0.1)
        countdown = 1
        received_data = b""
        while not received_data and countdown > 0:
            if ser.in_waiting > 0:
                while True:
                    byte = ser.read(1)
                    received_data += byte
                    if byte == b"\n":
                        break
            countdown -= 0.2
            if countdown == 0:
                break
            sleep(0.1)

        print("Received data:", received_data.decode().rstrip())
        break
