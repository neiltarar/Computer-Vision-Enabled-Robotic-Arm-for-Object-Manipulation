import depthai as dai
from time import sleep

def check_available_devices():
    available_devices = dai.Device.getAllAvailableDevices()
    # if no OAK-D camera is not connected raise error
    if len(available_devices) == 0:
        print("No devices found!")
        sleep(2)
        check_available_devices()
    else:
        print(f"Found {len(available_devices)} device(s)")
        my_device = dai.Device()
        return my_device