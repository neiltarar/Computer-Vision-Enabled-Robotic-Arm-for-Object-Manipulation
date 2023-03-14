import UserInput from './UserInput/UserInput';
import {useState} from 'react'
import './App.css';

function App() {
  const [command, setCommand] = useState('')
  
  const handleChange = (event) => {
    setCommand(event.target.value)
  }  

  const handleClick = () => {
    console.log(command)
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
    <div className="App">
     <h1>User Interface For the Flask App</h1>
     <UserInput 
      handleChange = {handleChange}
      handleClick = {handleClick}
     />
    </div>
  );
}

export default App;
