import depthai as dai
from domain.model.object_detection import HandTracker

class Pipeline:
    def __init__(self) -> None:
        self.create_pipeline()

    def create_pipeline(self):
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

        # connect to the rgb camera on the device
        print("Creating RGB Cam")
        self.rgb_cam.setBoardSocket(dai.CameraBoardSocket.RGB)
        self.rgb_cam.setInterleaved(False)
        self.rgb_cam.setPreviewKeepAspectRatio(False)

        # create xlinkout(s)
        self.xlink_rgb_out = self.pipeline.create(dai.node.XLinkOut)
        self.xlink_rgb_out.setStreamName("frame")

        # link nodes
        self.link_nodes(self.rgb_cam, self.xlink_rgb_out)

    def link_nodes(self, rgb_cam, xlink_rgb_out):
        print("linking nodes")
        rgb_cam.preview.link(xlink_rgb_out.input)

