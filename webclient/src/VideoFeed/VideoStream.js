import React from 'react';

function VideoStream() {
  console.log(process.env.REACT_APP_ENV)
  return (
    <div>
      {process.env.REACT_APP_ENV == "development" ? 
      <img src="http://localhost:5000/video_feed" alt="http://localhost:5000/video_feed"></img>
      :
      <img src="http://localhost:5000/video_feed" alt="http://localhost:5000/video_feed"></img>
    }
    </div>
  );
}

export default VideoStream;