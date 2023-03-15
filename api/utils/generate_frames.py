from flask import Response
import depthai as dai
from utils.check_available_devices import check_available_devices
from domain.model.pipelines.Pipeline import Pipeline
from time import sleep
import cv2
import base64
import imutils

check_available_devices()
pipeline = Pipeline()


def generate_frames():
    with dai.Device(pipeline.pipeline) as device:
        frame_queue = device.getOutputQueue(name="frame", maxSize=1, blocking=False)
        sleep(0.01)
        while True:
            frame = frame_queue.get()
            cvFrame = frame.getCvFrame()
            _, img_encoded = cv2.imencode('.jpg', cvFrame)
            yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(img_encoded) + b'\r\n')
        
        device.close
        cv2.destroyAllWindows()
   