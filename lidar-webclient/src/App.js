import {Header} from './components/Header'
import {Home} from './components/Home'
import {Connection} from './components/Connection';
import {Fragment, useState} from 'react';

function App() {
  
  return (
    <Fragment>
      <Connection />
      <Header />
      <Home />
    </Fragment>
  );
}

export default App;
