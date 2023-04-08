import { Flex, Box } from "@chakra-ui/react";
import ComputerVisionVideoFeed from "./VideoFeed/ComputerVisionVideoFeed";
import RobotArmVideoFeed from "./VideoFeed/RobotArmVideoStream";

const VideoStreams = () => {
    return (
        <Flex
            margin='auto'
            alignItems='center'
        >
            <Box
                w='50%'
                p={1}
                pl={100}
            >
                <ComputerVisionVideoFeed />
            </Box>
            <Box
                w="50%"
                p={1}
            >
                <RobotArmVideoFeed />
            </Box>
        </Flex>
    );
};

export default VideoStreams;
