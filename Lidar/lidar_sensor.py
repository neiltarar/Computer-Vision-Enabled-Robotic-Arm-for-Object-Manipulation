from rplidar import RPLidar
lidar = RPLidar('/dev/ttyUSB0')

info = lidar.get_info()
print(info)

health = lidar.get_health()
print(health)

def get_lidar_readings():
    reading = []
    for i, scan in enumerate(lidar.iter_scans()):
        if i > 10:
            break
        reading.append(len(scan))

    lidar.stop()
    lidar.disconnect()
    return reading