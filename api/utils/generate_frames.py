from flask import Response
import depthai as dai
from utils.check_available_devices import check_available_devices
from domain.model.pipelines.Pipeline import Pipeline
from time import sleep
import numpy as np
import cv2

check_available_devices()
pipeline = Pipeline()


def generate_frames():
    with dai.Device(pipeline.pipeline) as device:
        calib_data = device.readCalibration()
        calib_lens_pos = calib_data.getLensPosition(dai.CameraBoardSocket.RGB)
        print(f"RGB calibration lens position: {calib_lens_pos}")
        pipeline.rgb_cam.initialControl.setManualFocus(calib_lens_pos)
        frame_queue = device.getOutputQueue(name="frame", maxSize=1, blocking=False)
        palm_det_queue = device.getOutputQueue(name= "palm_det_out" , maxSize=1, blocking=False)
        sleep(0.01)
        while True:
            frame = frame_queue.get()
            palm_det = palm_det_queue.get()
            score = np.array(palm_det.getLayerFp16("classificators"), dtype=np.float16)
            # bboxes = np.array(palm_det.getLayerFp16("regressors"), dtype=np.float16).reshape((self.nb_anchors, 18))
            cvFrame = frame.getCvFrame()
            _, img_encoded = cv2.imencode('.jpg', cvFrame)
            yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(img_encoded) + b'\r\n')
        
        device.close
        cv2.destroyAllWindows()
   