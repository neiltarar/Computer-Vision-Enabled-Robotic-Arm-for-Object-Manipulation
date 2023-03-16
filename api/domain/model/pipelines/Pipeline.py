import depthai as dai
from domain.model.object_detection import HandTracker
from pathlib import Path

class Pipeline:
    def __init__(self) -> None:
        self.create_pipeline()


    def create_pipeline(self):
        self.SCRIPT_DIR = Path(__file__).resolve().parent
        self.PALM_DETECTION_MODEL = str(self.SCRIPT_DIR / "../../../ai_models/palm_detection_sh4.blob")
        self.LANDMARK_MODEL_FULL = str(self.SCRIPT_DIR / "../../../ai_models/hand_landmark_full_sh4.blob")
        self.LANDMARK_MODEL_LITE = str(self.SCRIPT_DIR / "../../../ai_models/hand_landmark_lite_sh4.blob")
        self.LANDMARK_MODEL_SPARSE = str(self.SCRIPT_DIR / "../../../ai_models/hand_landmark_sparse_sh4.blob")
        self.pipeline = dai.Pipeline()
        self.pipeline.setOpenVINOVersion(version = dai.OpenVINO.Version.VERSION_2022_1)
        
        # RGB Camera setup
        self.rgb_cam = self.pipeline.createColorCamera()
        self.rgb_cam.setColorOrder(dai.ColorCameraProperties.ColorOrder.BGR)
        self.rgb_cam.setResolution(dai.ColorCameraProperties.SensorResolution.THE_4_K)
        self.rgb_cam.setInterleaved(False)
        self.rgb_cam.setFps(23)
        self.rgb_cam.setPreviewSize(780, 600)

        # Stereo Camera setup
        self.mono_resolution = dai.MonoCameraProperties.SensorResolution.THE_400_P
        self.mono_left = self.pipeline.createMonoCamera()
        self.mono_left.setBoardSocket(dai.CameraBoardSocket.LEFT)
        self.mono_left.setResolution(self.mono_resolution)
        self.mono_left.setFps(23)

        self.mono_right = self.pipeline.createMonoCamera()
        self.mono_right.setBoardSocket(dai.CameraBoardSocket.RIGHT)
        self.mono_right.setResolution(self.mono_resolution)
        self.mono_right.setFps(23)
        
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

        # NN Models
        # Palm detection model
        self.palm_det_model = self.pipeline.createNeuralNetwork()
        self.palm_det_model.setBlobPath(self.PALM_DETECTION_MODEL)

        # Image manipulation Nodes
        self.palm_det_manip_node = self.pipeline.create(dai.node.ImageManip)
        self.palm_det_manip_node.initialConfig.setResize(128,128)

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
                        self.palm_det_model,
                        self.palm_det_manip_node,
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
                   palm_det_model,
                   palm_det_manip_node,
                   xlink_rgb_out,
                   xlink_spatial_data_out,
                   xlink_spatial_calc_config_in,
                   xlink_palm_det_out):
        print("linking nodes")
        rgb_cam.preview.link(xlink_rgb_out.input)
        rgb_cam.preview.link(palm_det_manip_node.inputImage)
        palm_det_manip_node.out.link(palm_det_model.input)
        mono_left.out.link(stereo.left)
        mono_right.out.link(stereo.right)

        stereo.depth.link(spatial_location_calculator.inputDepth)
        spatial_location_calculator.out.link(xlink_spatial_data_out.input)
        xlink_spatial_calc_config_in.out.link(spatial_location_calculator.inputConfig)

        palm_det_model.out.link(xlink_palm_det_out.input)
