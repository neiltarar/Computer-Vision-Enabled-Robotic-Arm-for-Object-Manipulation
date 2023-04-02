import time
import threading
import depthai as dai
import utils.mediapipe_utils as mp


from domain.model.cv_draw.DrawOnDetection import DrawOnDetection
from utils.check_available_devices import check_available_devices
from utils.gesture_conversion_utils.convert_hand_x_to_robot_yaw import convert_hand_yaw_to_robot_yaw
from domain.model.pipelines.Pipeline import Pipeline
from utils.gesture_conversion_utils.convert_hand_y_to_robot_arm2_tilt import convert_hand_y_to_robot_arm2_tilt
from utils.serial_communication import send_receive_serial_data
from time import sleep
import marshal
import cv2


def generate_frames(lm_threshold):
    lm_threshold = lm_threshold
    device = check_available_devices()
    pipeline = Pipeline(device)
    draw = DrawOnDetection()

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
        robot_claw = 0
        count = 0

        while True:
            start_time = time.time()
            frame, palm_det_data = None, None

            while time.time() - start_time < timeout_sec:
                try:
                    frame = frame_queue.tryGet()
                except RuntimeError:
                    frame = None

                try:
                    palm_det_data = palm_det_queue.tryGet()
                except RuntimeError:
                    palm_det_data = None

                if frame is not None and palm_det_data is not None:
                    break
                else:
                    sleep(0.01)

            if frame is not None and palm_det_data is not None:
                robot_commands = []
                cvFrame = frame.getCvFrame()
                palm_det_data = marshal.loads(palm_det_data.getData())
                for i in range(len(palm_det_data.get("lm_score", []))):
                    hand = pipeline.extract_hand_data(palm_det_data, i)
                    if hand.lm_score > lm_threshold:
                        mp.recognize_gesture(hand)
                        # hand_x, hand_y, hand_z = hand.xyz
                        hand_x, hand_y, hand_z = [coord / 10 for coord in hand.xyz]
                        draw.draw_hand(hand, cvFrame)
                        robot_yaw = convert_hand_yaw_to_robot_yaw(hand_x, sensitivity=0, rounding_multiple=6)
                        robot_tilt_arm2 = convert_hand_y_to_robot_arm2_tilt(hand_y, sensitivity=0, rounding_multiple=6)
                        if hand.gesture == "FIST":
                            robot_claw = 0
                        elif hand.gesture == "FIVE":
                            robot_claw = 150

                        robot_commands.extend([f"BASE1-{robot_yaw}", f"ARM2-{robot_tilt_arm2}",f"CLAW7-{robot_claw}\n"])
                        count += 0.1
                        if count > 3:
                            total_command = ",".join(robot_commands)
                            threading.Thread(target=send_receive_serial_data, args=(total_command,)).start()
                            count = 0
                        # if count > 1.3:
                        #     threading.Thread(target=send_receive_serial_data, args=(f"ARM2-{robot_tilt_arm2}",)).start()
                        #     count = 0
                _, img_encoded = cv2.imencode('.jpg', cvFrame)
                yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(img_encoded) + b'\r\n')

            else:
                sleep(0.1)
    except Exception as e:
        print(f"ERROR OCCURRED: \n{e}")
        generate_frames(lm_threshold)

    device.close()
    cv2.destroyAllWindows()