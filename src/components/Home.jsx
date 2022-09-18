import {useState} from 'react';

export const Home = (props) => {
	const [counter, setCounter] = useState(0)
	
	return(
		<main>
			<h1>Robot Control Page</h1>
			<h5>
				Counter:<span>{counter}</span>
			</h5>
			<button onClick={()=>{
				setCounter(counter + 1)
				console.log('clicked ' + counter)
			}}>Add</button>
		</main>
	)
}