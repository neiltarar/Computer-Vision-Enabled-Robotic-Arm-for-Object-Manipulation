import ROSLIB from "roslib";
import {useEffect, useState} from 'react';
import {cleanup} from '@testing-library/react';

export const Connection = (props) => {
	const [connected, setConnected] = useState(false);
	
	console.log(connected);
	useEffect(() => {
		const ros = new ROSLIB.Ros();
		ros.on("connection", () => {
			console.log("Connection established");
			setConnected(true);
		});
		
		ros.on("close", () => {
			console.log('connection closed');
			setConnected(false);
			setTimeout(() => {
				try {
					ros.connect("ws://192.168.64.1:9090");
				} catch (err) {
					console.log(err);
				}
			}, 5000);
		});
		
		try {
			ros.connect("ws://192.168.64.1:9090");
		} catch (err) {
			console.log(err);
		}
		
		return function cleanup() {
			ros.on("close", () => {
				console.log('connection closed');
			});
		};
		
	}, [connected]);
	
	return (
		<h1>{connected ? <p>Connected</p> : <p>Disconnected</p>}</h1>
	);
};