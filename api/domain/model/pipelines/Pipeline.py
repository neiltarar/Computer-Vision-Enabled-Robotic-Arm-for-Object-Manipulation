import depthai as dai
import numpy as np
from utils.mediapipe_utils import HandRegion, rotated_rect_to_points, recognize_gesture
from utils.build_manager_script import build_manager_script
from utils.mediapipe_utils import find_isp_scale_params
from pathlib import Path


class Pipeline:
    def __init__(self, device) -> None:
        self.rgb_cam = None
        self.pipeline = None
        self.palm_det_input_length = 128
        self.SCRIPT_DIR = Path(__file__).resolve().parent
        self.PALM_DETECTION_MODEL = str(self.SCRIPT_DIR / "../../../ai_models/palm_detection_sh4.blob")
        self.PALM_DETECTION_POST_PROCESSING_MODEL = str(self.SCRIPT_DIR / "../../../ai_models/PDPostProcessing_top2_sh1.blob")
        self.LANDMARK_MODEL_FULL = str(self.SCRIPT_DIR / "../../../ai_models/hand_landmark_full_sh4.blob")
        self.LANDMARK_MODEL_LITE = str(self.SCRIPT_DIR / "../../../ai_models/hand_landmark_lite_sh4.blob")
        self.LANDMARK_MODEL_SPARSE = str(self.SCRIPT_DIR / "../../../ai_models/hand_landmark_sparse_sh4.blob")
        self.pd_score_thresh = 0.5
        self.pd_nms_thresh = 0.3
        self.lm_score_thresh = 0.5
        self.solo = False
        self.lm_nb_threads = 2
        assert self.lm_nb_threads in [1, 2]
        self.lm_nb_threads = self.lm_nb_threads
        self.crop = False
        self.xyz = True
        self.use_world_landmarks = False
        self.stats = False
        self.trace = 0
        self.gesture = False
        self.use_handedness_average = True
        self.single_hand_tolerance_thresh = 10
        self.use_same_image = True
        self.internal_frame_height = 640
        self.resolution = (1920, 1080)

        self.width, self.scale_nd = find_isp_scale_params(self.internal_frame_height * self.resolution[0] / self.resolution[1], self.resolution, is_height=False)
        self.img_h = int(round(self.resolution[1] * self.scale_nd[0] / self.scale_nd[1]))
        self.img_w = int(round(self.resolution[0] * self.scale_nd[0] / self.scale_nd[1]))
        self.pad_h = (self.img_w - self.img_h) // 2
        self.pad_w = 0
        self.frame_size = self.img_w
        self.crop_w = 0

        self.hand_landmark_length = 224
        self.device = device
        self.input_type = "rgb"  # OAK* internal color camera
        self.laconic = None == "rgb_laconic"  # Camera frames are not sent to the host
        self.use_gesture = False
        self.resolution = (1920, 1080)

        self.internal_fps = 29
        self.video_fps = self.internal_fps  # Used when saving the output in a video file. Should be close to the real fps
        self.create_pipeline()

    def create_pipeline(self):
        self.pipeline = dai.Pipeline()
        self.pipeline.setOpenVINOVersion(version=dai.OpenVINO.Version.VERSION_2022_1)

        # RGB Camera setup
        self.rgb_cam = self.pipeline.createColorCamera()
        self.rgb_cam.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
        self.rgb_cam.setColorOrder(dai.ColorCameraProperties.ColorOrder.BGR)
        self.rgb_cam.setInterleaved(False)
        self.rgb_cam.setIspScale(self.scale_nd[0], self.scale_nd[1])
        self.rgb_cam.setFps(self.internal_fps)
        self.rgb_cam.setVideoSize(self.img_w, self.img_h)
        self.rgb_cam.setPreviewSize(self.img_w, self.img_h)

        # Stereo Camera setup
        self.mono_resolution = dai.MonoCameraProperties.SensorResolution.THE_400_P
        self.mono_left = self.pipeline.createMonoCamera()
        self.mono_left.setBoardSocket(dai.CameraBoardSocket.LEFT)
        self.mono_left.setResolution(self.mono_resolution)
        self.mono_left.setFps(self.internal_fps)

        self.mono_right = self.pipeline.createMonoCamera()
        self.mono_right.setBoardSocket(dai.CameraBoardSocket.RIGHT)
        self.mono_right.setResolution(self.mono_resolution)
        self.mono_right.setFps(self.internal_fps)

        self.stereo = self.pipeline.createStereoDepth()
        self.stereo.setConfidenceThreshold(230)
        # LR-check is required for depth alignment
        self.stereo.setLeftRightCheck(True)
        self.stereo.setDepthAlign(dai.CameraBoardSocket.RGB)
        self.stereo.setSubpixel(False)  # subpixel True -> latency

        self.spatial_location_calculator = self.pipeline.createSpatialLocationCalculator()
        self.spatial_location_calculator.setWaitForConfigInput(True)
        self.spatial_location_calculator.inputDepth.setBlocking(False)
        self.spatial_location_calculator.inputDepth.setQueueSize(1)

        # connect to the rgb camera on the device
        print("Creating RGB Cam")
        self.rgb_cam.setBoardSocket(dai.CameraBoardSocket.RGB)
        self.rgb_cam.setInterleaved(False)
        self.rgb_cam.setPreviewKeepAspectRatio(False)

        # Create Script Nodes
        self.script = self.pipeline.create(dai.node.Script)
        self.script.setScript(build_manager_script())

        # NN Models
        # Palm detection model
        self.palm_det_model = self.pipeline.createNeuralNetwork()
        self.palm_det_model.setBlobPath(self.PALM_DETECTION_MODEL)

        # Pode detection post processing "model"
        self.post_palm_det_model = self.pipeline.create(dai.node.NeuralNetwork)
        self.post_palm_det_model.setBlobPath(self.PALM_DETECTION_POST_PROCESSING_MODEL)

        # Landmark model
        self.landmark_model = self.pipeline.create(dai.node.NeuralNetwork)
        self.landmark_model.setBlobPath(self.LANDMARK_MODEL_LITE)
        self.landmark_model.setNumInferenceThreads(2)

        # Image manipulation Nodes
        self.palm_det_manip_node = self.pipeline.create(dai.node.ImageManip)
        self.palm_det_manip_node.setMaxOutputFrameSize(128*128*3)
        self.palm_det_manip_node.setWaitForConfigInput(True)
        self.palm_det_manip_node.inputImage.setQueueSize(1)
        self.palm_det_manip_node.inputImage.setBlocking(False)

        self.pre_hand_landmark_manip_node = self.pipeline.create(dai.node.ImageManip)
        self.pre_hand_landmark_manip_node.setMaxOutputFrameSize(self.hand_landmark_length * self.hand_landmark_length * 3)
        self.pre_hand_landmark_manip_node.setWaitForConfigInput(True)
        self.pre_hand_landmark_manip_node.inputImage.setQueueSize(1)
        self.pre_hand_landmark_manip_node.inputImage.setBlocking(False)
        # create xlinkout(s)
        self.xlink_rgb_out = self.pipeline.create(dai.node.XLinkOut)
        self.xlink_rgb_out.setStreamName("frame")

        self.xlink_spatial_data_out = self.pipeline.createXLinkOut()
        self.xlink_spatial_data_out.setStreamName("spatial_data_out")

        self.xlink_palm_det_out = self.pipeline.createXLinkOut()
        self.xlink_palm_det_out.setStreamName("palm_det_out")

        # create xlinkin(s)
        self.xlink_spatial_calc_config_in = self.pipeline.createXLinkIn()
        self.xlink_spatial_calc_config_in.setStreamName("spatial_calc_config_in")
        # link nodes
        self.link_nodes(self.rgb_cam,
                        self.mono_left,
                        self.mono_right,
                        self.stereo,
                        self.spatial_location_calculator,
                        self.script,
                        self.palm_det_model,
                        self.post_palm_det_model,
                        self.landmark_model,
                        self.palm_det_manip_node,
                        self.pre_hand_landmark_manip_node,
                        self.xlink_rgb_out,
                        self.xlink_spatial_data_out,
                        self.xlink_spatial_calc_config_in,
                        self.xlink_palm_det_out)

    def link_nodes(self,
                   rgb_cam,
                   mono_left,
                   mono_right,
                   stereo,
                   spatial_location_calculator,
                   script,
                   palm_det_model,
                   post_palm_det_model,
                   landmark_model,
                   palm_det_manip_node,
                   pre_hand_landmark_manip_node,
                   xlink_rgb_out,
                   xlink_spatial_data_out,
                   xlink_spatial_calc_config_in,
                   xlink_palm_det_out):
        print("linking nodes")
        mono_left.out.link(stereo.left)
        mono_right.out.link(stereo.right)

        stereo.depth.link(spatial_location_calculator.inputDepth)
        script.outputs['spatial_location_config'].link(spatial_location_calculator.inputConfig)
        spatial_location_calculator.out.link(script.inputs['spatial_data'])
        spatial_location_calculator.out.link(xlink_spatial_data_out.input)
        xlink_spatial_calc_config_in.out.link(spatial_location_calculator.inputConfig)

        rgb_cam.video.link(xlink_rgb_out.input)
        rgb_cam.preview.link(palm_det_manip_node.inputImage)
        script.outputs['pre_pd_manip_cfg'].link(palm_det_manip_node.inputConfig)
        rgb_cam.preview.link(pre_hand_landmark_manip_node.inputImage)
        script.outputs['pre_lm_manip_cfg'].link(pre_hand_landmark_manip_node.inputConfig)
        palm_det_manip_node.out.link(palm_det_model.input)

        pre_hand_landmark_manip_node.out.link(landmark_model.input)
        landmark_model.out.link(script.inputs['from_lm_nn'])

        palm_det_model.out.link(post_palm_det_model.input)
        post_palm_det_model.out.link(script.inputs['from_post_pd_nn'])

        script.outputs['host'].link(xlink_palm_det_out.input)

    def extract_hand_data(self, res, hand_idx):
        hand = HandRegion()
        hand.rect_x_center_a = res["rect_center_x"][hand_idx] * self.frame_size
        hand.rect_y_center_a = res["rect_center_y"][hand_idx] * self.frame_size
        hand.rect_w_a = hand.rect_h_a = res["rect_size"][hand_idx] * self.frame_size
        hand.rotation = res["rotation"][hand_idx]
        hand.rect_points = rotated_rect_to_points(hand.rect_x_center_a, hand.rect_y_center_a, hand.rect_w_a, hand.rect_h_a, hand.rotation)
        hand.lm_score = res["lm_score"][hand_idx]
        hand.handedness = res["handedness"][hand_idx]
        hand.label = "right" if hand.handedness > 0.5 else "left"
        hand.norm_landmarks = np.array(res['rrn_lms'][hand_idx]).reshape(-1,3)
        hand.landmarks = (np.array(res["sqn_lms"][hand_idx]) * self.frame_size).reshape(-1,2).astype(np.int32)
        if self.xyz:
            hand.xyz = np.array(res["xyz"][hand_idx])
            hand.xyz_zone = res["xyz_zone"][hand_idx]
        # If we added padding to make the image square, we need to remove this padding from landmark coordinates and from rect_points
        if self.pad_h > 0:
            hand.landmarks[:,1] -= self.pad_h
            for i in range(len(hand.rect_points)):
                hand.rect_points[i][1] -= self.pad_h
        if self.pad_w > 0:
            hand.landmarks[:,0] -= self.pad_w
            for i in range(len(hand.rect_points)):
                hand.rect_points[i][0] -= self.pad_w

        # World landmarks
        if self.use_world_landmarks:
            hand.world_landmarks = np.array(res["world_lms"][hand_idx]).reshape(-1, 3)

        if self.use_gesture: recognize_gesture(hand)

        return hand
