import UserInput from './UserInput/UserInput';
import VideoStream from './VideoFeed/VideoStream';
import {useState} from 'react'
import './App.css';

function App() {
  const [command, setCommand] = useState('')
  
  const handleChange = (event) => {
    setCommand(event.target.value)
  }  

  const handleKeyPress = (event) => {
    if(event.keyCode ==13) {
      fetch('/command', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ command })
    })
      .then(response => response.json())
      .then(data => console.log(data))
      .catch(error => console.error(error));
    } 
  }

  const handleClick = () => {
    fetch('/command', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ command })
    })
      .then(response => response.json())
      .then(data => console.log(data))
      .catch(error => console.error(error));
  }

  return (
    <>
      <div className="App">
        <h1>User Interface For the Flask App</h1>
        <UserInput 
          handleChange = {handleChange}
          handleClick = {handleClick}
          handleKeyPress = {handleKeyPress}
        />
        <div>
          <h1>Live Video Stream</h1>
          <VideoStream />
        </div>
      </div>
    </>
  );
}

export default App;
