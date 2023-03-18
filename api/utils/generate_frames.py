import time

import depthai as dai
from utils.check_available_devices import check_available_devices
from utils.convert_hand_yaw_to_robot_yaw import convert_hand_yaw_to_robot_yaw
from domain.model.pipelines.Pipeline import Pipeline
from time import sleep
import marshal
import cv2


def generate_frames():
    device = check_available_devices()
    pipeline = Pipeline(device)
    try:
        device.startPipeline(pipeline.pipeline)
        calib_data = device.readCalibration()
        calib_lens_pos = calib_data.getLensPosition(dai.CameraBoardSocket.RGB)
        print(f"RGB calibration lens position: {calib_lens_pos}")
        pipeline.rgb_cam.initialControl.setManualFocus(calib_lens_pos)
        frame_queue = device.getOutputQueue(name="frame", maxSize=1, blocking=False)
        palm_det_queue = device.getOutputQueue(name="palm_det_out", maxSize=1, blocking=False)

        timeout_ms = 50
        timeout_sec = timeout_ms / 1000

        while True:
            start_time = time.time()
            frame, palm_det_data = None, None

            while time.time() - start_time < timeout_sec:
                try:
                    frame = frame_queue.get()
                except RuntimeError:
                    frame = None

                try:
                    palm_det_data = palm_det_queue.get()
                except RuntimeError:
                    palm_det_data = None

                if frame is not None and palm_det_data is not None:
                    break
                else:
                    sleep(0.01)

            if frame is not None and palm_det_data is not None:
                palm_det_data = marshal.loads(palm_det_data.getData())
                hands = []
                for i in range(len(palm_det_data.get("lm_score", []))):
                    hand = pipeline.extract_hand_data(palm_det_data, i)
                    robot_yaw = convert_hand_yaw_to_robot_yaw(hand.rotation, sensitivity=0, rounding_multiple=6)

                    if robot_yaw is not None:
                        print(f"Robot yaw angle: {robot_yaw} \t hand_y:{hand.rotation}")
                    else:
                        print("Yaw value out of range.")
                    hands.append(hand)
                cvFrame = frame.getCvFrame()
                _, img_encoded = cv2.imencode('.jpg', cvFrame)
                yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(img_encoded) + b'\r\n')

            else:
                sleep(0.01)
    except Exception as e:
        print(f"ERROR OCCURRED: \n{e}")
        generate_frames()

    device.close()
    cv2.destroyAllWindows()

#
# if __name__ == "__main__":
#     for _ in generate_frames():
#         pass