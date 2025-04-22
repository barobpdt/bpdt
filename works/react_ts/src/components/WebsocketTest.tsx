import { useEffect, useRef, useState } from 'react';
import Loader from './Loader';

export default function WebsocketTest() { 
  const ws = useRef<WebSocket | null>(null);
  const socketUrl = 'ws://localhost:8092' 
  const [recvData, setRecvData] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  
  useEffect(() => {
    ws.current = new WebSocket(socketUrl);
    ws.current.onopen = () => {
      console.log('WebSocket connection opened.')
      setIsLoading(true);
    }
    ws.current.onmessage = (event) => {
		console.log("@@ on message event=> ", event)
      if (event.data) {
        // const base64ImageData = 'data:image/jpg;base64,' + event.data;
        setCctvData(event.data);
      }
    };
    ws.current.onerror = () => console.log('WebSocket Error')
    ws.current.onclose = () => console.log('Websocket connection is closed') 

    return () => {
      if (ws.current && ws.current.readyState === 1) {
        ws.current.close();
      }
    };
  }, []);

  return (
    <div className='cctv-container relative'> 
		{recvData}
    </div>
  );
}