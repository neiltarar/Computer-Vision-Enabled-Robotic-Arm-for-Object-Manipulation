import depthai as dai

class Pipeline:
    def __init__(self) -> None:
        self.create_pipeline()

    def create_pipeline(self):
        self.pipeline = dai.Pipeline()
        self.rgb_cam = self.pipeline.createColorCamera()
        self.rgb_cam.setColorOrder(dai.ColorCameraProperties.ColorOrder.BGR)
        self.rgb_cam.setResolution(dai.ColorCameraProperties.SensorResolution.THE_4_K)
        self.rgb_cam.setInterleaved(False)
        self.rgb_cam.setPreviewSize(780, 600)

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

