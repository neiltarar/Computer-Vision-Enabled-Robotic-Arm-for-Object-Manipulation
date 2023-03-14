import serial
from time import sleep

def send_receive_serial_data(command, port="/dev/ttyS0", baudrate=9600):
    ser = serial.Serial(port, baudrate, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                        bytesize=serial.EIGHTBITS, timeout=0.1)

    while True:
        data = "#" + command + "\n"
        ser.write(data.encode())
        print("Sent data:", data)
        sleep(0.5)
        countdown = 1
        received_data = b""
        while not received_data and countdown > 0:
            if ser.in_waiting > 0:
                while True:
                    byte = ser.read(1)
                    received_data += byte
                    if byte == b"\n":
                        break
            countdown -= 0.5
            if countdown == 0:
                break
            sleep(0.5)
        
        print("Received data:", received_data.decode().rstrip())
        break
