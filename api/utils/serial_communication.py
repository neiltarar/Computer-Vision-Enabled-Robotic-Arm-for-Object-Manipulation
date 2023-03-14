import serial
from time import sleep

ser = serial.Serial ("/dev/ttyS0", 9600, parity=serial.PARITY_NONE,stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS, timeout=0.1)    #Open port with baud rate


while True:
 #   if ser.in_waiting > 0:
    user_input = input("Enter data to send: ")
    data = "#" + user_input + "\n"
    ser.write(data.encode())
    print("Sent data:", data)
    sleep(0.5)
    # Read data if available
    countdown = 1
    received_data = b""
    while not received_data and countdown > 0:
        print(received_data)
        print(countdown)
        if ser.in_waiting > 0:
            while True:
                # Read one byte at a time
                byte = ser.read(1)
                received_data += byte
                if byte == b"\n":
                    # End of message, break out of loop
                    break
        countdown -= 0.5
        if countdown == 0:
            break
        sleep(0.5)
    
    print("Received data:", received_data.decode().rstrip())
    sleep(2)

