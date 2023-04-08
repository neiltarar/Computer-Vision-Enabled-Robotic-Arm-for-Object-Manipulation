import React from 'react';
import UserInput from './UserInput/UserInput';
import { useState } from 'react';
import { Flex, Text } from '@chakra-ui/react';
import VideoSteams from './Components/VideoSteams';

function App() {
  const [command, setCommand] = useState('');

  const handleChange = (event) => {
    setCommand(event.target.value);
  };

  const handleKeyPress = (event) => {
    if (event.keyCode === 13) {
      fetch('/command', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ command }),
      })
          .then((response) => response.json())
          .then((data) => console.log(data))
          .catch((error) => console.error(error));
    }
  };

  const handleClick = () => {
    fetch('/command', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ command }),
    })
        .then((response) => response.json())
        .then((data) => console.log(data))
        .catch((error) => console.error(error));
  };

  return (
        <Flex
            flexDir='column'
            alignItems='center'
            textAlign='center'
        >
          <Text
              p={55}
          >
            User Interface For the Flask App
          </Text>
          <UserInput
              handleChange={handleChange}
              handleClick={handleClick}
              handleKeyPress={handleKeyPress}
          />
          <Text
              p={50}
          >
            Live Video Stream
          </Text>
          <VideoSteams />
        </Flex>
  );
}

export default App;
